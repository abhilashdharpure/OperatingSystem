#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "process.h"
#include "pmm.h"
#include "arch/x86_64/gdt.h"
#include "debug.h"
#include "paging.h"
#include "syscall/sys_execve.h"
#include "auxv.h"
#include "errno.h"



#define PT_LOAD           1

#define PF_X  (1 << 0)
#define PF_W  (1 << 1)
#define PF_R  (1 << 2)

extern Process *current_process;

extern uint64_t *kernel_pml4_virt;

static void exec_abort(Process *p, uint64_t *old_pd, uint64_t old_cr3) {
    if (p->cr3 != old_cr3) {                 /* new PD was installed: drop it */
        free_user_address_space(p->cr3);
        p->page_directory = old_pd; p->cr3 = old_cr3;
    }
}
#define EXEC_FAIL() do { exec_abort(p, old_pd, old_cr3); kfree(ph); return -ENOEXEC; } while (0)


/* ---------- small helpers ---------- */

void debug_check_va(Process *p, uint64_t va)
{
    uint64_t pa = get_mapped_phys(p->page_directory, va);
    log_info("CHKVA", "VA=0x%llx -> PA=0x%llx", (unsigned long long)va, (unsigned long long)pa);
    dump_pte_for_va(p->cr3, va);
}


static uint64_t compute_phdr_addr_from_mem(Elf64_Ehdr *eh, void *data)
{
    Elf64_Phdr *ph = (Elf64_Phdr *)((uint8_t *)data + eh->e_phoff);

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t off   = eh->e_phoff;
        uint64_t p_off = ph[i].p_offset;
        uint64_t p_end = ph[i].p_offset + ph[i].p_filesz;

        if (off >= p_off && off < p_end) {
            uint64_t delta = off - p_off;
            return ph[i].p_vaddr + delta;
        }
    }

    return 0;
}

void debug_dump_user_bytes(Process *p, uint64_t va, size_t n)
{
    uint64_t pa = get_mapped_phys(p->page_directory, va);
    log_info("DBG", "user va=0x%llu -> pa=0x%llx", va, pa);
    if (!pa) return;

    uint64_t page_base = pa & ~(PAGE_SIZE - 1);
    uint64_t offset    = va & (PAGE_SIZE - 1);

    uint8_t *kva = (uint8_t *)phys_to_virt(page_base);
    kva += offset;

    // for (size_t i = 0; i < n; ++i) {
    //     log_info("DBG", "  %llx: %d",
    //              (unsigned long long)(va + i),
    //              kva[i]);
    // }
}

static inline void u64_store(Process *p, uint64_t va, uint64_t val)
{
    // log_info("EXEC", "u64_store: va=0x%llx val=0x%llx", va, val);
    uint64_t pa = get_mapped_phys(p->page_directory, va);
    // log_info("EXEC", "u64_store: va=0x%llx -> pa=0x%llx", va, pa);
    if (!pa) {
        log_critical("EXEC", "u64_store: unmapped user VA=0x%llu", va);
        return;
    }

    uint64_t page_base = pa & ~(PAGE_SIZE - 1);
    uint64_t offset    = va & (PAGE_SIZE - 1);

    uint8_t *kva = (uint8_t *)phys_to_virt(page_base);
    kva += offset;
    *(uint64_t *)kva = val;
}

void debug_dump_user_stack(Process *p, uint64_t sp)
{
    log_info("USTACK", "Dumping initial user stack at RSP=0x%llu", sp);

    for (int i = 0; i < 20; i++) {
        uint64_t va = sp + i*8;
        uint64_t pa = get_mapped_phys(p->page_directory, va);
        uint64_t val = 0;

        if (pa) {
            uint64_t page_base = pa & ~(PAGE_SIZE - 1);
            uint64_t offset    = va & (PAGE_SIZE - 1);
            uint8_t *kva = (uint8_t *)phys_to_virt(page_base);
            kva += offset;
            val = *(uint64_t *)kva;
        }

        log_info("USTACK", "  [0x%llx] -> PA=0x%llx : 0x%llx",
                 va, pa, val);
    }
}

void dump_process_regs(Process *p)
{
    log_info("PROC", "RIP=%llx RSP=%llx", p->regs.rip, p->regs.rsp);
}
static int apply_elf_relocations(Process *p, void *data, Elf64_Ehdr *eh) {
    Elf64_Phdr *ph = (Elf64_Phdr *)((uint8_t *)data + eh->e_phoff);
    Elf64_Dyn *dyn = NULL;
    size_t dyn_size = 0;

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn *)((uint8_t *)data + ph[i].p_offset);
            dyn_size = ph[i].p_filesz / sizeof(Elf64_Dyn);
            break;
        }
    }
    if (!dyn) return 0;

    Elf64_Rela *rela = NULL;
    size_t rela_count = 0;
    for (size_t i = 0; i < dyn_size; i++) {
        if (dyn[i].d_tag == DT_RELA)
            rela = (Elf64_Rela *)((uint8_t *)data + dyn[i].d_un.d_ptr);
        else if (dyn[i].d_tag == DT_RELASZ)
            rela_count = dyn[i].d_un.d_val / sizeof(Elf64_Rela);
    }
    if (!rela || rela_count == 0) return 0;

    // Compute base
    uint64_t base_addr = 0;
    if (eh->e_type == ET_DYN) {
        base_addr = UINT64_MAX;
        for (int i = 0; i < eh->e_phnum; i++) {
            if (ph[i].p_type == PT_LOAD && ph[i].p_vaddr < base_addr)
                base_addr = ph[i].p_vaddr;
        }
        base_addr &= ~0xFFFULL;
    }

    // Apply relocations
    for (size_t i = 0; i < rela_count; i++) {
        Elf64_Rela *r = &rela[i];
        if (ELF64_R_TYPE(r->r_info) == R_X86_64_RELATIVE) {
            uint64_t target_va = r->r_offset;
            uint64_t value     = (eh->e_type == ET_DYN) ? base_addr + r->r_addend
                                                       : r->r_addend;

            uint64_t pa = get_mapped_phys(p->page_directory, target_va);
            if (pa) {
                uint64_t *kva = (uint64_t *)phys_to_virt((pa & ~(PAGE_SIZE-1)) +
                                                         (target_va & (PAGE_SIZE-1)));
                *kva = value;
                log_info("RELOC", "VA=0x%llx patched to 0x%llx", target_va, value);
            }
        }
    }
    return 0;
}


/* Translate a virtual address inside a PT_LOAD segment into a file offset */
static off_t va_to_file_offset(Elf64_Phdr *phdrs, int phnum, uint64_t va) {
    for (int i = 0; i < phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD) continue;

        uint64_t start = phdrs[i].p_vaddr;
        uint64_t end   = phdrs[i].p_vaddr + phdrs[i].p_filesz;

        if (va >= start && va < end) {
            return (off_t)(phdrs[i].p_offset + (va - start));
        }
    }
    return -1; /* not found */
}
static int apply_elf_relocations_fd(Process *p, int fd, Elf64_Ehdr *eh, Elf64_Phdr *phdrs)
{
    Elf64_Dyn *dyn = NULL;
    size_t dyn_count = 0;

    /* 1. Read PT_DYNAMIC */
    for (int i = 0; i < eh->e_phnum; i++) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            size_t bytes = phdrs[i].p_filesz;
            dyn_count = bytes / sizeof(Elf64_Dyn);

            dyn = (Elf64_Dyn *)kmalloc(bytes);
            if (!dyn) return -ENOMEM;

            if (VFS_Lseek(fd, (off_t)phdrs[i].p_offset, SEEK_SET) < 0) { kfree(dyn); return -EFAULT; }
            if (VFS_Read(fd, dyn, bytes) != (int)bytes) { kfree(dyn); return -EFAULT; }
            break;
        }
    }
    if (!dyn) return 0;

    /* 2. Find relocation info */
    uint64_t rela_va = 0, rel_va = 0;
    size_t rela_count = 0, rel_count = 0;
    for (size_t i = 0; i < dyn_count; i++) {
        switch (dyn[i].d_tag) {
        case DT_RELA:    rela_va    = dyn[i].d_un.d_ptr; break;
        case DT_RELASZ:  rela_count = dyn[i].d_un.d_val / sizeof(Elf64_Rela); break;
        case DT_REL:     rel_va     = dyn[i].d_un.d_ptr; break;
        case DT_RELSZ:   rel_count  = dyn[i].d_un.d_val / sizeof(Elf64_Rel); break;
        }
    }

    /* 3. Compute base address */
    uint64_t base_addr = 0;
    if (eh->e_type == ET_DYN) {
        base_addr = UINT64_MAX;
        for (int i = 0; i < eh->e_phnum; i++) {
            if (phdrs[i].p_type == PT_LOAD && phdrs[i].p_vaddr < base_addr)
                base_addr = phdrs[i].p_vaddr;
        }
        base_addr &= ~0xFFFULL;
    }

    /* 4. Handle RELA */
    if (rela_va && rela_count) {
        off_t rela_off = va_to_file_offset(phdrs, eh->e_phnum, rela_va);
        if (rela_off >= 0) {
            Elf64_Rela *rela = (Elf64_Rela *)kmalloc(rela_count * sizeof(Elf64_Rela));
            if (rela) {
                if (VFS_Lseek(fd, rela_off, SEEK_SET) >= 0 &&
                    VFS_Read(fd, rela, rela_count * sizeof(Elf64_Rela)) == (int)(rela_count * sizeof(Elf64_Rela))) {
                    for (size_t i = 0; i < rela_count; i++) {
                        Elf64_Rela *r = &rela[i];
                        if (ELF64_R_TYPE(r->r_info) == R_X86_64_RELATIVE) {
                            uint64_t target_va = r->r_offset;
                            uint64_t pa = get_mapped_phys(p->page_directory, target_va);
                            if (pa) {
                                uint64_t *kva = (uint64_t *)phys_to_virt((pa & ~(PAGE_SIZE-1)) +
                                                                         (target_va & (PAGE_SIZE-1)));
                                uint64_t newval = (eh->e_type == ET_DYN) ? base_addr + r->r_addend
                                                                         : r->r_addend;
                                log_info("RELOC", "RELA VA=0x%llx old=0x%llx new=0x%llx",
                                         target_va, *kva, newval);
                                *kva = newval;
                            }
                        }
                    }
                }
                kfree(rela);
            }
        }
    }

    /* 5. Handle REL */
    if (rel_va && rel_count) {
        off_t rel_off = va_to_file_offset(phdrs, eh->e_phnum, rel_va);
        if (rel_off >= 0) {
            Elf64_Rel *rel = (Elf64_Rel *)kmalloc(rel_count * sizeof(Elf64_Rel));
            if (rel) {
                if (VFS_Lseek(fd, rel_off, SEEK_SET) >= 0 &&
                    VFS_Read(fd, rel, rel_count * sizeof(Elf64_Rel)) == (int)(rel_count * sizeof(Elf64_Rel))) {
                    for (size_t i = 0; i < rel_count; i++) {
                        Elf64_Rel *r = &rel[i];
                        if (ELF64_R_TYPE(r->r_info) == R_X86_64_RELATIVE) {
                            uint64_t target_va = r->r_offset;
                            uint64_t pa = get_mapped_phys(p->page_directory, target_va);
                            if (pa) {
                                uint64_t *kva = (uint64_t *)phys_to_virt((pa & ~(PAGE_SIZE-1)) +
                                                                         (target_va & (PAGE_SIZE-1)));
                                uint64_t old = *kva;
                                uint64_t newval = (eh->e_type == ET_DYN) ? base_addr + old : old;
                                log_info("RELOC", "REL VA=0x%llx old=0x%llx new=0x%llx",
                                         target_va, old, newval);
                                *kva = newval;
                            }
                        }
                    }
                }
                kfree(rel);
            }
        }
    }

    kfree(dyn);
    return 0;
}


static int map_user_stack(Process *p)
{
    uint64_t stack_bottom = USER_STACK_TOP - USER_STACK_SIZE;
    // const uint64_t flags = PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_NX;
    const uint64_t flags = PAGE_PRESENT | PAGE_RW | PAGE_USER;

    for (uint64_t va = stack_bottom; va < USER_STACK_TOP; va += PAGE_SIZE) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            log_critical("STACK", "Failed to allocate page for user stack");
            return -1;
        }

        uint8_t *kva = (uint8_t *)phys_to_virt(pa);
        memset(kva, 0, PAGE_SIZE);

        if (map_page(p->page_directory, va, pa, flags) != 0) {
            log_critical("STACK", "map_page failed for user stack VA=0x%llx", va);
            return -1;
        }
    }

    log_info("STACK", "Mapped user stack: [0x%llx, 0x%llx)",
             stack_bottom, USER_STACK_TOP);
    return 0;
}

static int map_elf_segments_from_mem(Process *p,
                                     void *data,
                                     Elf64_Ehdr *eh)
{
    Elf64_Phdr *ph = (Elf64_Phdr *)((uint8_t *)data + eh->e_phoff);
    const uint64_t user_rw_flags = PAGE_PRESENT | PAGE_RW | PAGE_USER;

    log_info("ELF", "map_elf_segments_from_mem: e_phnum=%u e_phoff=0x%llx",
             eh->e_phnum, (unsigned long long)eh->e_phoff);

    for (int _i = 0; _i < eh->e_phnum; _i++) {
        log_info("ELF", " PHDR[%d]: type=%u off=0x%llx vaddr=0x%llx filesz=0x%llx memsz=0x%llx flags=0x%x",
                 _i,
                 (unsigned)ph[_i].p_type,
                 (unsigned long long)ph[_i].p_offset,
                 (unsigned long long)ph[_i].p_vaddr,
                 (unsigned long long)ph[_i].p_filesz,
                 (unsigned long long)ph[_i].p_memsz,
                 (unsigned)ph[_i].p_flags);
    }

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t seg_flags = PAGE_PRESENT | PAGE_USER;
        if (ph[i].p_flags & PF_W)
            seg_flags |= PAGE_RW;
        if (!(ph[i].p_flags & PF_X))
            seg_flags |= PAGE_NX;

        uint64_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
        uint64_t last_byte = ph[i].p_vaddr + ph[i].p_memsz - 1;
        uint64_t seg_end   = (last_byte & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) return -ENOMEM;

            memset(phys_to_virt(pa), 0, PAGE_SIZE);

            if (map_page(p->page_directory, va, pa, seg_flags) != 0)
                return -EFAULT;

            uint64_t offset_in_segment = va - seg_start;
            if (offset_in_segment < ph[i].p_filesz) {
                uint64_t to_copy = PAGE_SIZE;
                if (offset_in_segment + to_copy > ph[i].p_filesz)
                    to_copy = ph[i].p_filesz - offset_in_segment;

                memcpy(phys_to_virt(pa),
                    (uint8_t*)data + ph[i].p_offset + offset_in_segment,
                    to_copy);
            }
        }
    }

    uint64_t tls_pa =
    get_mapped_phys(p->page_directory, 0x400059b8);

    log_info("TLSCHK",
            "0x400059b8 -> PA=0x%llx",
            tls_pa);


    return 0;
}


// --- shared helpers -------------------------------------------------

static uint64_t compute_phdr_addr_from_fd(Elf64_Ehdr *eh, Elf64_Phdr *ph)
{
    // Try normal case first
    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t off   = eh->e_phoff;
        uint64_t p_off = ph[i].p_offset;
        uint64_t p_end = ph[i].p_offset + ph[i].p_filesz;

        if (off >= p_off && off < p_end) {
            uint64_t delta = off - p_off;
            return ph[i].p_vaddr + delta;
        }
    }

    // Fallback: if headers are outside PT_LOAD, assume they were mapped at 0x40000000
    return 0x40000000 + eh->e_phoff;
}


static void build_initial_stack(Process *p,
                                Elf64_Ehdr *eh,
                                uint64_t phdr_addr,
                                size_t argc,
                                char **argv,
                                char **envp)
{
    uint64_t sp = USER_STACK_TOP;

    uint64_t arg_addrs[MAX_EXEC_ARGS];
    uint64_t env_addrs[MAX_EXEC_ARGS];

    /* -------------------------
     * limit args
     * ------------------------- */
    if (argc > MAX_EXEC_ARGS)
        argc = MAX_EXEC_ARGS;

    size_t envc = 0;
    if (envp) {
        while (envp[envc] && envc < MAX_EXEC_ARGS)
            envc++;
    }

    /* =========================================================
     * 1. COPY STRINGS (env first, then argv)
     * ========================================================= */

    // ENV strings
    for (int64_t i = (int64_t)envc - 1; i >= 0; i--) {
        size_t len = strlen(envp[i]) + 1;
        size_t padded = (len + 7) & ~7ULL;

        sp -= padded;
        env_addrs[i] = sp;

        for (size_t off = 0; off < len; off += 8) {
            uint64_t word = 0;
            memcpy(&word, envp[i] + off, (len - off >= 8) ? 8 : (len - off));
            u64_store(p, sp + off, word);
        }
    }

    // ARGV strings
    for (int64_t i = (int64_t)argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        size_t padded = (len + 7) & ~7ULL;

        sp -= padded;
        arg_addrs[i] = sp;

        for (size_t off = 0; off < len; off += 8) {
            uint64_t word = 0;
            memcpy(&word, argv[i] + off, (len - off >= 8) ? 8 : (len - off));
            u64_store(p, sp + off, word);
        }
    }

    /* =========================================================
     * 2. AUXV EXTRA DATA
     * ========================================================= */

    const char platform[] = "x86_64";

    sp -= 16;
    uint64_t random_va = sp;
    u64_store(p, random_va + 0, 0x123456789ABCDEF0ULL);
    u64_store(p, random_va + 8, 0x0FEDCBA987654321ULL);

    sp -= ((sizeof(platform) + 7) & ~7ULL);
    uint64_t platform_va = sp;
    for (size_t i = 0; i < sizeof(platform); i += 8) {
        uint64_t word = 0;
        memcpy(&word, platform + i, sizeof(platform) - i >= 8 ? 8 : sizeof(platform) - i);
        u64_store(p, sp + i, word);
    }

    /* =========================================================
     * 3. ALIGNMENT (CRITICAL FIX)
     * ========================================================= */

    // MUST ensure (%rsp + 8) % 16 == 0 at entry
    sp &= ~0xFULL;
    sp -= 8;

    /* =========================================================
     * 4. BUILD AUXV
     * ========================================================= */

    uint64_t auxv[64];
    int ax = 0;

    if (phdr_addr) {
        auxv[ax++] = AT_PHDR; auxv[ax++] = phdr_addr;
    }
    auxv[ax++] = AT_PHENT; auxv[ax++] = eh->e_phentsize;
    auxv[ax++] = AT_PHNUM; auxv[ax++] = eh->e_phnum;
    auxv[ax++] = AT_PAGESZ; auxv[ax++] = 4096;
    auxv[ax++] = AT_ENTRY; auxv[ax++] = eh->e_entry;
    auxv[ax++] = AT_PLATFORM; auxv[ax++] = platform_va;
    auxv[ax++] = AT_RANDOM; auxv[ax++] = random_va;
    auxv[ax++] = AT_SECURE; auxv[ax++] = 0;
    auxv[ax++] = AT_NULL; auxv[ax++] = 0;

    /* =========================================================
     * 5. PUSH STACK (Linux order)
     * ========================================================= */

    // auxv (reverse)
    for (int i = ax - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, auxv[i]);
    }

    // envp NULL
    sp -= 8;
    u64_store(p, sp, 0);

    // envp pointers
    for (int64_t i = (int64_t)envc - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, env_addrs[i]);
    }

    // argv NULL
    sp -= 8;
    u64_store(p, sp, 0);

    // argv pointers
    for (int64_t i = (int64_t)argc - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, arg_addrs[i]);
    }

    // argc
    sp -= 8;
    u64_store(p, sp, argc);

    p->regs.rsp = sp;
}


/* ---------- exec from memory ---------- */

pid_t exec_elf_mem(void *data,
                   size_t size,
                   BootParams *bootParams,
                   size_t argc,
                   char **argv,
                   char **envp)
{
    (void)size;

    log_info("EXEC", "exec_elf_mem start");
    Elf64_Ehdr *eh = (Elf64_Ehdr *)data;

    if (eh->e_ident[EI_MAG0] != ELFMAG0 ||
        eh->e_ident[EI_MAG1] != ELFMAG1 ||
        eh->e_ident[EI_MAG2] != ELFMAG2 ||
        eh->e_ident[EI_MAG3] != ELFMAG3) {
        log_critical("EXEC", "Not an ELF file");
        return -1;
    }
    if (eh->e_ident[EI_CLASS] != ELFCLASS64) {
        log_critical("EXEC", "Not an ELF64 file");
        return -1;
    }
    if (eh->e_machine != EM_X86_64) {
        log_critical("EXEC", "Not x86_64 ELF (e_machine=%u)", eh->e_machine);
        return -1;
    }
    if (eh->e_entry == 0) {
        log_critical("EXEC", "ELF has no entry");
        return -1;
    }

    log_info("EXEC", "exec_elf_mem process_create");
    Process *p = process_create("user");
    if (!p)
        return -1;

    proc_register(p);

    current_process = p;

    log_info("EXEC", "exec_elf_mem create_user_pd");
    page_dir_t pd = create_user_pd();
    p->page_directory = pd.pd_virt;
    p->cr3            = pd.pd_phys;
    p->mmap_base      = USER_MMAP_BASE;
    p->brk_start      = USER_HEAP_START;
    p->brk_end        = USER_HEAP_START;
    p->brk_cur        = USER_HEAP_START;
    p->brk_end_limit  = USER_HEAP_END;

    log_info("EXEC", "exec_elf_mem clone_kernel_mappings");
    clone_kernel_mappings_for_user(p->page_directory);

    MemoryRegion *user_region = NULL;
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        if (bootParams->Memory.Regions[i].Type == 1 &&
            bootParams->Memory.Regions[i].Begin >= 0x100000) {
            user_region = &bootParams->Memory.Regions[i];
            break;
        }
    }
    if (!user_region) {
        log_critical("EXEC", "No user memory region available");
        return -1;
    }

    log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x",
             user_region->Begin, user_region->Length, user_region->Type);

    if (map_elf_segments_from_mem(p, data, eh) != 0) {
        log_critical("EXEC", "map_elf_segments_from_mem failed");
        return -1;
    }
    log_info("EXEC", "map_elf_segments_from_mem done");


    if (map_user_stack(p) != 0)
    {
        log_critical("EXEC", "map_user_stack failed");
        return -1;
    }
    log_info("EXEC", "map_user_stack done");


    uint64_t phdr_addr = compute_phdr_addr_from_mem(eh, data);
    if (!phdr_addr)
        phdr_addr = USER_START + eh->e_phoff;   // fallback

    log_info("EXEC", "exec_elf_mem: argc=%llu argv=%p envp=%p",
        (unsigned long long)argc, (void*)argv, (void*)envp);

    if (argv) {
        for (size_t i = 0; i < argc; i++) {
            log_info("EXEC", "  argv[%llu]=%p",
                    (unsigned long long)i, (void*)argv[i]);
        }
    }
    if (envp) {
        for (size_t i = 0; envp[i] && i < 8; i++) {
            log_info("EXEC", "  envp[%llu]=%p",
                    (unsigned long long)i, (void*)envp[i]);
        }
    }


    log_info("EXEC", "Before build_initial_stack: phdr_addr=0x%llx", phdr_addr);
    build_initial_stack(p, eh, phdr_addr, argc, argv, envp);
    log_info("EXEC", "After build_initial_stack: RSP=0x%llx", p->regs.rsp);
    debug_dump_user_stack(p, p->regs.rsp);

    uint64_t rip_pa = get_mapped_phys(p->page_directory, eh->e_entry);
    uint64_t rsp_pa = get_mapped_phys(p->page_directory, p->regs.rsp);

    log_info("EXEC", "Process regs: RIP=0x%llx RSP=0x%llx",
             (unsigned long long)eh->e_entry,
             (unsigned long long)p->regs.rsp);
    log_info("EXEC", "RIP VA=0x%llx -> PA=0x%llx",
             eh->e_entry, rip_pa);
    log_info("EXEC", "RSP VA=0x%llx -> PA=0x%llx",
             p->regs.rsp, rsp_pa);

    if (!rip_pa || !rsp_pa)
        log_critical("EXEC", "ELF pages not mapped!");

    log_info("EXEC", "ELF64 loaded entry=0x%llx", eh->e_entry);

    p->regs.rip    = eh->e_entry;
    //p->regs.rsp    = /* already set by build_initial_stack */;
    p->regs.rflags = 0x202;          // IF=1
    p->regs.cs     = USER_CODE_SELECTOR;
    p->regs.ss     = USER_DATA_SELECTOR;

    dump_process_regs(p);
    enter_user_mode_from_process(p);
    return 0;
}

/* ---------- exec from fd ---------- */
// pid_t exec_elf_load(int fd,
//                        BootParams *bootParams,
//                        size_t argc,
//                        char **argv,
//                        char **envp)

int exec_elf_load(Process *p, int fd, BootParams *bootParams,
                  size_t argc, char **argv, char **envp, uint64_t *old_cr3_out)
{
    log_info("EXEC", "exec_elf_load: start");

    /* -----------------------------
     * 1. Read ELF header
     * ----------------------------- */
    Elf64_Ehdr eh;
    if (VFS_Lseek(fd, 0, SEEK_SET) < 0) {
        log_critical("EXEC", "exec_elf_load: lseek(0) failed");
        return -1;
    }

    int n = VFS_Read(fd, &eh, sizeof(eh));
    if (n != (int)sizeof(eh)) {
        log_critical("EXEC", "exec_elf_load: short read on ELF header (%d)", n);
        return -1;
    }

    if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
        eh.e_ident[EI_MAG1] != ELFMAG1 ||
        eh.e_ident[EI_MAG2] != ELFMAG2 ||
        eh.e_ident[EI_MAG3] != ELFMAG3) {
        log_critical("EXEC", "exec_elf_load: not an ELF file");
        return -1;
    }
    if (eh.e_ident[EI_CLASS] != ELFCLASS64) {
        log_critical("EXEC", "exec_elf_load: not ELF64");
        return -1;
    }
    if (eh.e_machine != EM_X86_64) {
        log_critical("EXEC", "exec_elf_load: not x86_64 (e_machine=%u)", eh.e_machine);
        return -1;
    }
    if (eh.e_entry == 0) {
        log_critical("EXEC", "exec_elf_load: ELF has no entry");
        return -1;
    }

    /* -----------------------------
     * 2. Read program headers
     * ----------------------------- */
    if (eh.e_phnum == 0 || eh.e_phentsize != sizeof(Elf64_Phdr)) {
        log_critical("EXEC", "exec_elf_load: invalid phdr table (phnum=%u entsize=%u)",
                     eh.e_phnum, eh.e_phentsize);
        return -1;
    }

    size_t phdr_bytes = eh.e_phnum * sizeof(Elf64_Phdr);
    Elf64_Phdr *ph = (Elf64_Phdr *)kmalloc(phdr_bytes);
    if (!ph) {
        log_critical("EXEC", "exec_elf_load: kmalloc(phdrs) failed");
        return -1;
    }
    uint64_t old_cr3 = p->cr3;
    uint64_t *old_pd = p->page_directory;
    
    if (VFS_Lseek(fd, (off_t)eh.e_phoff, SEEK_SET) < 0) {
        log_critical("EXEC", "exec_elf_load: lseek to phoff failed");
        EXEC_FAIL();
    }

    n = VFS_Read(fd, ph, phdr_bytes);
    if (n != (int)phdr_bytes) {
        log_critical("EXEC", "exec_elf_load: short read on phdr table (%d/%zu)", n, phdr_bytes);
        EXEC_FAIL();
    }

    for (int i = 0; i < eh.e_phnum; i++) {
        log_info("ELF", " PHDR[%d]: type=%u off=0x%llx vaddr=0x%llx filesz=0x%llx memsz=0x%llx flags=0x%x",
                 i,
                 (unsigned)ph[i].p_type,
                 (unsigned long long)ph[i].p_offset,
                 (unsigned long long)ph[i].p_vaddr,
                 (unsigned long long)ph[i].p_filesz,
                 (unsigned long long)ph[i].p_memsz,
                 (unsigned)ph[i].p_flags);
    }

    /* -----------------------------
     * 3. Create process + page tables
     * ----------------------------- */
    // Process *p = process_create("user");
    // if (!p) {
    //     log_critical("EXEC", "exec_elf_load: process_create failed");
    //     kfree(ph);
    //     return -1;
    // }

    // current_process = p;

    // uint64_t old_cr3 = p->cr3;
    // uint64_t *old_pd = p->page_directory;
    p->fs_base = 0; p->gs_base = 0;

    page_dir_t pd = create_user_pd();
    if (!pd.pd_phys || !pd.pd_virt) {
        log_critical("EXEC", "exec_elf_load: create_user_pd failed");
        EXEC_FAIL();
    }

    p->page_directory = pd.pd_virt;
    p->cr3            = pd.pd_phys;
    p->mmap_base      = USER_MMAP_BASE;
    p->brk_start      = USER_HEAP_START;
    p->brk_end        = USER_HEAP_START;
    p->brk_cur        = USER_HEAP_START;
    p->brk_end_limit  = USER_HEAP_END;

    clone_kernel_mappings_for_user(p->page_directory);

    /* Optional: pick a user region from bootParams, like exec_elf_mem does */
    MemoryRegion *user_region = NULL;
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        if (bootParams->Memory.Regions[i].Type == 1 &&
            bootParams->Memory.Regions[i].Begin >= 0x100000) {
            user_region = &bootParams->Memory.Regions[i];
            break;
        }
    }
    if (!user_region) {
        log_critical("EXEC", "exec_elf_load: no user memory region");
        EXEC_FAIL();
    }

    log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x",
             user_region->Begin, user_region->Length, user_region->Type);

    /* -----------------------------
     * 4. Map PT_LOAD segments from fd
     * ----------------------------- */
    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        Elf64_Phdr *seg = &ph[i];

        uint64_t vaddr      = seg->p_vaddr;
        uint64_t seg_start  = vaddr & ~(PAGE_SIZE - 1);
        uint64_t last_byte  = vaddr + seg->p_memsz - 1;
        uint64_t seg_end    = (last_byte & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

        uint64_t seg_flags  = PAGE_PRESENT | PAGE_USER;
        if (seg->p_flags & PF_W)
            seg_flags |= PAGE_RW;
        if (seg->p_flags & PF_X)
            seg_flags &= ~PAGE_NX;
        else
            seg_flags |= PAGE_NX;

        log_info("ELF", "PT_LOAD[%d]: vaddr=0x%llx filesz=0x%llx memsz=0x%llx",
                 i,
                 (unsigned long long)seg->p_vaddr,
                 (unsigned long long)seg->p_filesz,
                 (unsigned long long)seg->p_memsz);

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_critical("EXEC", "exec_elf_load: out of pages mapping segment");
                EXEC_FAIL();
            }

            uint8_t *kva = (uint8_t *)phys_to_virt(pa);
            memset(kva, 0, PAGE_SIZE);

            if (map_page(p->page_directory, va, pa, seg_flags) != 0) {
                log_critical("EXEC", "exec_elf_load: map_page failed for VA=0x%llx", va);
                EXEC_FAIL();
            }

            /* Copy file contents into this page if within p_filesz */
            uint64_t offset_in_segment = va - seg_start;
            if (offset_in_segment < seg->p_filesz) {
                uint64_t to_copy = PAGE_SIZE;
                if (offset_in_segment + to_copy > seg->p_filesz)
                    to_copy = seg->p_filesz - offset_in_segment;

                off_t file_off = (off_t)(seg->p_offset + offset_in_segment);
                if (VFS_Lseek(fd, file_off, SEEK_SET) < 0) {
                    log_critical("EXEC", "exec_elf_load: lseek to segment data failed");
                    kfree(ph);
                    return -1;
                }

                int r = VFS_Read(fd, kva, (size_t)to_copy);
                if (r != (int)to_copy) {
                    log_critical("EXEC", "exec_elf_load: short read in segment (%d/%llu)",
                                 r, (unsigned long long)to_copy);
                    EXEC_FAIL();
                }
            }
        }

        log_info("ELF", "Mapped PT_LOAD segment %d: VA [0x%llx, 0x%llx)",
                 i, (unsigned long long)seg_start, (unsigned long long)seg_end);
    }

    // /* >>> ADD PATCH HERE <<< */
    // /* Explicitly map the ELF header page (offset 0–0x1000) */
    // {
    //     uint64_t va = 0x40000000;        // start of user text
    //     uint64_t pa = pmm_alloc_page();
    //     if (!pa) {
    //         log_critical("EXEC", "Failed to allocate page for ELF header");
    //         EXEC_FAIL();
    //     }

    //     uint8_t *kva = (uint8_t *)phys_to_virt(pa);
    //     memset(kva, 0, PAGE_SIZE);

    //     if (VFS_Lseek(fd, 0, SEEK_SET) < 0) {
    //         log_critical("EXEC", "lseek to ELF start failed");
    //         EXEC_FAIL();
    //     }
    //     int r = VFS_Read(fd, kva, PAGE_SIZE);
    //     if (r < 0) {
    //         log_critical("EXEC", "read ELF header page failed");
    //         EXEC_FAIL();
    //     }

    //     const uint64_t flags = PAGE_PRESENT | PAGE_USER | PAGE_RW;
    //     if (map_page(p->page_directory, va, pa, flags) != 0) {
    //         log_critical("EXEC", "map_page failed for ELF header VA=0x%llx", va);
    //         EXEC_FAIL();
    //     }

    //     log_info("EXEC", "Mapped ELF header page at VA=0x%llx", va);
    // }

    /* -----------------------------
     * 5. Map user stack
     * ----------------------------- */
    if (map_user_stack(p) != 0) {
        log_critical("EXEC", "exec_elf_load: map_user_stack failed");
        EXEC_FAIL();
    }

    /* -----------------------------
     * 6. Build initial stack (argc/argv/envp/auxv)
     * ----------------------------- */
    // uint64_t phdr_addr = compute_phdr_addr_from_fd(&eh, ph);
    // if (!phdr_addr)
    //     phdr_addr = USER_START + eh.e_phoff; // fallback

    uint64_t phdr_addr = compute_phdr_addr_from_fd(&eh, ph);

    log_info("EXEC", "Using phdr_addr=0x%llx", phdr_addr);

    log_info("EXEC", "exec_elf_load: argc=%llu argv=%p envp=%p",
             (unsigned long long)argc, (void*)argv, (void*)envp);

    if (argv) {
        for (size_t i = 0; i < argc; i++) {
            log_info("EXEC", "  argv[%llu]=%p",
                     (unsigned long long)i, (void*)argv[i]);
        }
    }
    if (envp) {
        for (size_t i = 0; envp[i] && i < 8; i++) {
            log_info("EXEC", "  envp[%llu]=%p",
                     (unsigned long long)i, (void*)envp[i]);
        }
    }

    log_info("EXEC", "Before build_initial_stack: phdr_addr=0x%llx", phdr_addr);
    build_initial_stack(p, &eh, phdr_addr, argc, argv, envp);
    log_info("EXEC", "After build_initial_stack: RSP=0x%llx", p->regs.rsp);

    /* -----------------------------
     * 7. Finalize regs and jump
     * ----------------------------- */
    uint64_t rip_pa = get_mapped_phys(p->page_directory, eh.e_entry);
    uint64_t rsp_pa = get_mapped_phys(p->page_directory, p->regs.rsp);

    log_info("EXEC", "Process regs: RIP=0x%llx RSP=0x%llx",
             (unsigned long long)eh.e_entry,
             (unsigned long long)p->regs.rsp);
    log_info("EXEC", "RIP VA=0x%llx -> PA=0x%llx",
             (unsigned long long)eh.e_entry, (unsigned long long)rip_pa);
    log_info("EXEC", "RSP VA=0x%llx -> PA=0x%llx",
             (unsigned long long)p->regs.rsp, (unsigned long long)rsp_pa);

    if (!rip_pa || !rsp_pa) {
        log_critical("EXEC", "exec_elf_load: ELF pages not mapped!");
    }

    /* 8. Apply relocations */
    if (apply_elf_relocations_fd(p, fd, &eh, ph) != 0) 
    {
        log_critical("EXEC", "apply_elf_relocations_fd failed");
        EXEC_FAIL();
    }

    p->regs.rip    = eh.e_entry;
    p->regs.rflags = 0x202;
    p->regs.cs     = USER_CODE_SELECTOR;
    p->regs.ss     = USER_DATA_SELECTOR;

    // dump_process_regs(p);

    // kfree(ph);

    // enter_user_mode_from_process(p);

    // log_critical("EXEC", "exec_elf_load: returned unexpectedly from user mode");
    // return -1;

   kfree(ph);
   *old_cr3_out = old_cr3;
   return 0;
}
