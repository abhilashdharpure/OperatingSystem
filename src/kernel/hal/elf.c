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

// #define USER_STACK_SIZE   0x00200000ULL   // 2 MiB
#define PT_LOAD           1

#define PF_X  (1 << 0)
#define PF_W  (1 << 1)
#define PF_R  (1 << 2)

extern Process *current_process;

// // Must match linker script
// static const uint64_t USER_BASE      = 0x0000000040000000ULL;
// static const uint64_t USER_STACK_TOP = 0x0000000044000000ULL; // USER_BASE + 64 MiB

extern uint64_t *kernel_pml4_virt;

/* ---------- small helpers ---------- */

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
    log_info("DBG", "user va=0x%llx -> pa=0x%llx", va, pa);
    if (!pa) return;

    uint64_t page_base = pa & ~(PAGE_SIZE - 1);
    uint64_t offset    = va & (PAGE_SIZE - 1);

    uint8_t *kva = (uint8_t *)phys_to_virt(page_base);
    kva += offset;

    for (size_t i = 0; i < n; ++i) {
        log_info("DBG", "  %llx: %02x",
                 (unsigned long long)(va + i),
                 kva[i]);
    }
}

static inline void u64_store(Process *p, uint64_t va, uint64_t val)
{
    log_info("EXEC", "u64_store: va=0x%llx val=0x%llx", va, val);
    uint64_t pa = get_mapped_phys(p->page_directory, va);
    log_info("EXEC", "u64_store: va=0x%llx -> pa=0x%llx", va, pa);
    if (!pa) {
        log_critical("EXEC", "u64_store: unmapped user VA=0x%llx", va);
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
    log_info("USTACK", "Dumping initial user stack at RSP=0x%llx", sp);

    for (int i = 0; i < 20; i++) {
        uint64_t va = sp + i*8;
        uint64_t pa = get_mapped_phys(p->page_directory, va);
        uint64_t val = 0;

        if (pa) {
            uint8_t *kva = (uint8_t *)phys_to_virt(pa);
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

/* ---------- stack + mapping helpers ---------- */

static int map_user_stack(Process *p)
{
    uint64_t stack_bottom = USER_STACK_TOP - USER_STACK_SIZE;
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

        uint64_t vaddr = ph[i].p_vaddr;

        uint64_t seg_start = vaddr & ~(PAGE_SIZE - 1);
        uint64_t last_byte = vaddr + ph[i].p_memsz - 1;
        uint64_t seg_end   = (last_byte & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

        uint64_t seg_flags = user_rw_flags;
        if ((ph[i].p_flags & PF_X) && !(ph[i].p_flags & PF_W)) {
            // could later clear RW for RX segments
        }

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_critical("EXEC", "Out of pages while mapping ELF segment");
                return -1;
            }

            uint8_t *kva = (uint8_t *)phys_to_virt(pa);
            memset(kva, 0, PAGE_SIZE);

            if (map_page(p->page_directory, va, pa, seg_flags) != 0) {
                log_critical("EXEC", "map_page failed for ELF VA=0x%llx", va);
                return -1;
            }

            uint64_t offset_in_segment = va - seg_start;
            if (offset_in_segment < ph[i].p_filesz) {
                uint64_t to_copy = PAGE_SIZE;
                if (offset_in_segment + to_copy > ph[i].p_filesz)
                    to_copy = ph[i].p_filesz - offset_in_segment;

                memcpy(kva,
                       (uint8_t *)data + ph[i].p_offset + offset_in_segment,
                       to_copy);

                //debug_dump_user_bytes(p, va, 32);
            }
        }

        log_info("ELF", "Mapped PT_LOAD segment %d: VA [0x%llx, 0x%llx)",
                 i, seg_start, seg_end);
    }

    return 0;
}


// --- shared helpers -------------------------------------------------

static uint64_t compute_phdr_addr_from_fd(Elf64_Ehdr *eh, Elf64_Phdr *ph)
{
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

    log_info("EXEC", "build_initial_stack: sp=0x%llx argc=%llu argv=%p envp=%p",
             (unsigned long long)sp,
             (unsigned long long)argc,
             (void*)argv, (void*)envp);

    if (argc > MAX_EXEC_ARGS)
        argc = MAX_EXEC_ARGS;

    // Count envp
    size_t envc = 0;
    if (envp) {
        while (envp[envc] && envc < MAX_EXEC_ARGS)
            envc++;
    }
    log_info("EXEC", "build_initial_stack: envc=%llu",
             (unsigned long long)envc);

    // 1) Copy env strings (top‑down)
    for (int64_t i = (int64_t)envc - 1; i >= 0; i--) {
        const char *s = envp[i];
        size_t len = strlen(s);
        size_t total  = len + 1;
        size_t padded = (total + 7) & ~7ULL;

        sp -= padded;
        uint64_t str_va = sp;
        env_addrs[i] = str_va;

        log_info("EXEC", "ENV[%lld]=\"%s\" va=0x%llx total=%llu padded=%llu",
                 (long long)i, s,
                 (unsigned long long)str_va,
                 (unsigned long long)total,
                 (unsigned long long)padded);

        size_t off = 0;
        while (off < total) {
            uint64_t word = 0;
            for (int b = 0; b < 8 && off + b < total; b++) {
                ((uint8_t *)&word)[b] = (uint8_t)s[off + b];
            }
            u64_store(p, str_va + off, word);
            off += 8;
        }
    }

    // 2) Copy arg strings (top‑down)
    for (int64_t i = (int64_t)argc - 1; i >= 0; i--) {
        const char *s = argv[i];
        size_t len = strlen(s);
        size_t total  = len + 1;
        size_t padded = (total + 7) & ~7ULL;

        sp -= padded;
        uint64_t str_va = sp;
        arg_addrs[i] = str_va;

        log_info("EXEC", "ARG[%lld]=\"%s\" va=0x%llx total=%llu padded=%llu",
                 (long long)i, s,
                 (unsigned long long)str_va,
                 (unsigned long long)total,
                 (unsigned long long)padded);

        size_t off = 0;
        while (off < total) {
            uint64_t word = 0;
            for (int b = 0; b < 8 && off + b < total; b++) {
                ((uint8_t *)&word)[b] = (uint8_t)s[off + b];
            }
            u64_store(p, str_va + off, word);
            off += 8;
        }
    }

    // Align stack to 16 bytes before pushing pointers
    sp &= ~0xFULL;
    log_info("EXEC", "build_initial_stack: after strings, aligned sp=0x%llx",
             (unsigned long long)sp);

    // 3) Build auxv array in memory (we’ll push it last)
    uint64_t auxv[32];
    int ax = 0;

    if (phdr_addr) {
        auxv[ax++] = 3;          // AT_PHDR
        auxv[ax++] = phdr_addr;
    }

    auxv[ax++] = 4;              // AT_PHENT
    auxv[ax++] = eh->e_phentsize;

    auxv[ax++] = 5;              // AT_PHNUM
    auxv[ax++] = eh->e_phnum;

    auxv[ax++] = 6;              // AT_PAGESZ
    auxv[ax++] = 4096;

    auxv[ax++] = 9;              // AT_ENTRY
    auxv[ax++] = eh->e_entry;

    auxv[ax++] = 0;              // AT_NULL
    auxv[ax++] = 0;

    log_info("EXEC", "build_initial_stack: auxv entries=%d", ax / 2);

    // 4) Push auxv (last element first)
    for (int i = ax - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, auxv[i]);
    }

    // 5) envp NULL terminator
    sp -= 8;
    u64_store(p, sp, 0);

    // 6) envp pointers (envc-1 .. 0)
    for (int64_t i = (int64_t)envc - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, env_addrs[i]);
    }

    // 7) argv NULL terminator
    sp -= 8;
    u64_store(p, sp, 0);

    // 8) argv pointers (argc-1 .. 0)
    for (int64_t i = (int64_t)argc - 1; i >= 0; i--) {
        sp -= 8;
        u64_store(p, sp, arg_addrs[i]);
    }

    // 9) argc
    sp -= 8;
    u64_store(p, sp, argc);

    // SysV ABI: at entry, (%rsp + 8) must be 16‑byte aligned.
    // After pushing argc, we want (sp + 8) % 16 == 0.
    if (((sp + 8) & 0xF) != 0) {
        sp -= 8;
        u64_store(p, sp, 0); // padding
    }

    p->regs.rsp = sp;
    log_info("EXEC", "build_initial_stack: final RSP=0x%llx (entry=%llx)",
             (unsigned long long)p->regs.rsp,
             (unsigned long long)eh->e_entry);
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

    if (map_elf_segments_from_mem(p, data, eh) != 0)
        return -1;

    if (map_user_stack(p) != 0)
        return -1;

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

pid_t exec_elf_from_fd(int fd,
                       BootParams *bootParams,
                       size_t argc,
                       char **argv,
                       char **envp)
{
    Elf64_Ehdr eh;

    if (VFS_Read(fd, &eh, sizeof(eh)) != (int)sizeof(eh)) {
        log_critical("EXEC", "exec_elf_from_fd: failed to read ELF header");
        return -1;
    }

    if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
        eh.e_ident[EI_MAG1] != ELFMAG1 ||
        eh.e_ident[EI_MAG2] != ELFMAG2 ||
        eh.e_ident[EI_MAG3] != ELFMAG3 ||
        eh.e_ident[EI_CLASS] != ELFCLASS64 ||
        eh.e_machine != EM_X86_64 ||
        eh.e_entry == 0) {
        log_critical("EXEC", "exec_elf_from_fd: invalid or unsupported ELF");
        return -1;
    }

    size_t phdr_bytes = eh.e_phnum * sizeof(Elf64_Phdr);
    Elf64_Phdr *ph = (Elf64_Phdr *)kmalloc(phdr_bytes);
    if (!ph) {
        log_critical("EXEC", "exec_elf_from_fd: kmalloc(phdrs) failed");
        return -1;
    }

    if (VFS_Lseek(fd, (off_t)eh.e_phoff, 0) < 0) {
        log_critical("EXEC", "exec_elf_from_fd: seek to phdrs failed");
        kfree(ph);
        return -1;
    }

    if (VFS_Read(fd, ph, phdr_bytes) != (int)phdr_bytes) {
        log_critical("EXEC", "exec_elf_from_fd: read phdrs failed");
        kfree(ph);
        return -1;
    }

    log_info("ELF", "exec_elf_from_fd: e_phnum=%u e_phoff=0x%llx",
             eh.e_phnum, (unsigned long long)eh.e_phoff);
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

    log_info("EXEC", "exec_elf_from_fd process_create");
    Process *p = process_create("user");
    if (!p) {
        kfree(ph);
        return -1;
    }

    current_process = p;

    page_dir_t pd = create_user_pd();
    p->page_directory = pd.pd_virt;
    p->cr3            = pd.pd_phys;
    p->mmap_base      = USER_MMAP_BASE;
    p->brk_start      = USER_HEAP_START;
    p->brk_end        = USER_HEAP_START;
    p->brk_cur        = USER_HEAP_START;
    p->brk_end_limit  = USER_HEAP_END;

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
        kfree(ph);
        return -1;
    }

    log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x",
             user_region->Begin, user_region->Length, user_region->Type);

    if (map_user_stack(p) != 0) {
        kfree(ph);
        return -1;
    }

    // Map PT_LOAD segments
    const uint64_t user_rw_flags = PAGE_PRESENT | PAGE_RW | PAGE_USER;

    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t vaddr = ph[i].p_vaddr;
        uint64_t seg_start = vaddr & ~(PAGE_SIZE - 1);
        uint64_t last_byte = vaddr + ph[i].p_memsz - 1;
        uint64_t seg_end   = (last_byte & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

        uint64_t seg_flags = user_rw_flags;
        if ((ph[i].p_flags & PF_X) && !(ph[i].p_flags & PF_W)) {
            // you can later tighten to RX if you want
        }

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_critical("EXEC", "Out of pages while mapping ELF segment");
                kfree(ph);
                return -1;
            }

            uint8_t *kva = (uint8_t *)phys_to_virt(pa);
            memset(kva, 0, PAGE_SIZE);

            if (map_page(p->page_directory, va, pa, seg_flags) != 0) {
                log_critical("EXEC", "map_page failed for ELF VA=0x%llx", va);
                kfree(ph);
                return -1;
            }

            uint64_t offset_in_segment = va - seg_start;
            if (offset_in_segment < ph[i].p_filesz) {
                uint64_t to_copy = PAGE_SIZE;
                if (offset_in_segment + to_copy > ph[i].p_filesz)
                    to_copy = ph[i].p_filesz - offset_in_segment;

                off_t file_off = (off_t)(ph[i].p_offset + offset_in_segment);
                if (VFS_Lseek(fd, file_off, 0) < 0) {
                    log_critical("EXEC", "VFS_Lseek failed in PT_LOAD");
                    kfree(ph);
                    return -1;
                }

                int n = VFS_Read(fd, kva, to_copy);
                if (n != (int)to_copy) {
                    log_critical("EXEC", "VFS_Read short read in PT_LOAD (%d/%llu)",
                                 n, (unsigned long long)to_copy);
                    kfree(ph);
                    return -1;
                }
            }
        }
    }

    // Compute AT_PHDR
    uint64_t phdr_addr = compute_phdr_addr_from_fd(&eh, ph);
    if (!phdr_addr)
        phdr_addr = USER_START + eh.e_phoff; // fallback

    kfree(ph);

    // Build initial stack (argc/argv + auxv)
    build_initial_stack(p, &eh, phdr_addr, argc, argv, envp);

    p->regs.rip = eh.e_entry;

    log_info("EXEC", "ELF64 loaded entry=0x%llx", eh.e_entry);
    enter_user_mode_from_process(p);
    return 0;
}
