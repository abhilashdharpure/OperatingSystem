#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "process.h"
#include "pmm.h"
#include "debug.h"
#include "paging.h"

#define USER_STACK_SIZE 0x00010000U  // 64 KB
#define PT_LOAD 1

#define PF_X  (1 << 0)   // Execute
#define PF_W  (1 << 1)   // Write
#define PF_R  (1 << 2)   // Read


// User VA base (must match user linker script)
static const uint64_t USER_BASE      = 0x0000000040000000ULL;
static const uint64_t USER_STACK_TOP = 0x0000000040100000ULL; // USER_BASE + 1 MiB

extern uint64_t *kernel_pml4_virt;
void debug_dump_user_bytes(Process *p, uint64_t va, size_t n)
{
    uint64_t pa = get_mapped_phys(p->page_directory, va);
    log_info("DBG", "user va=0x%llx -> pa=0x%llx", va, pa);

    if (!pa) return;
    uint8_t *kva = (uint8_t*)(uintptr_t)pa;
    for (size_t i = 0; i < n; ++i) {
        log_info("DBG", "  %llx: %02x",
                 (unsigned long long)(va + i),
                 kva[i]);
    }
}

pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
{
    (void)size;

    log_info("EXEC", "exec_elf_mem start");
    Elf64_Ehdr *eh = (Elf64_Ehdr*)data;

    // Basic sanity: 64-bit ELF, x86_64
    if (eh->e_ident[0] != 0x7F || eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L'  || eh->e_ident[3] != 'F') {
        log_critical("EXEC", "Not an ELF file");
        return -1;
    }

    if (eh->e_entry == 0) {
        log_info("EXEC", "ELF has no entry");
        return -1;
    }

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

    log_info("EXEC", "exec_elf_mem process_create");

    Process *p = process_create("user");
    if (!p) {
        return -1;
    }

    log_info("EXEC", "exec_elf_mem create_user_pd");

    page_dir_t pd = create_user_pd();
    p->page_directory = pd.pd_virt;  // PML4 VA
    p->cr3            = pd.pd_phys;  // PML4 PA

    log_info("EXEC", "exec_elf_mem clone_kernel_mappings");
    // clone_kernel_mappings(p->page_directory);
    clone_kernel_mappings_for_user(p->page_directory);

    // Pick user memory region (for physical pages only)
    MemoryRegion* user_region = NULL;
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        if (bootParams->Memory.Regions[i].Type == 1 &&
            bootParams->Memory.Regions[i].Begin >= 0x100000)
        {
            user_region = &bootParams->Memory.Regions[i];
            break;
        }
    }
    if (!user_region) {
        log_info("EXEC", "No user memory region available");
        return -1;
    }

    log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x",
             user_region->Begin, user_region->Length, user_region->Type);

    // Stack in high user region (PML4[1])
    uint64_t stack_top    = USER_STACK_TOP;
    uint64_t stack_bottom = stack_top - USER_STACK_SIZE;

    log_info("ELF", "Stack Top == 0x%llx stack_bottom=0x%llx",
             stack_top, stack_bottom);

    // Map user stack pages: present, RW, user
    const uint64_t user_rw_flags = PAGE_PRESENT | PAGE_RW | PAGE_USER;

    for (uint64_t va = stack_bottom; va < stack_top; va += PAGE_SIZE) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            log_info("EXEC", "Out of pages for stack!");
            return -1;
        }

        uint8_t *kva = (uint8_t*)(uintptr_t)pa;
        memset(kva, 0, PAGE_SIZE);

        if (map_page(p->page_directory,
                     va,
                     pa,
                     user_rw_flags) != 0)
        {
            log_critical("EXEC", "map_page failed for stack VA=0x%llx", va);
            return -1;
        }
    }

    p->regs.rsp = stack_top;   // 64-bit stack pointer

    uint64_t esp_pa_dbg = get_mapped_phys(p->page_directory, p->regs.rsp - 8);
    log_info("STACK", "After map: RSP VA=0x%llx -> PA=0x%llx",
             p->regs.rsp, esp_pa_dbg);

    // Map ELF64 PT_LOAD segments as user pages
    Elf64_Phdr *ph = (Elf64_Phdr*)((uint8_t*)data + eh->e_phoff);
    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
        uint64_t last_byte = ph[i].p_vaddr + ph[i].p_memsz - 1;
        uint64_t seg_end = (last_byte & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

        log_info("ELF", "Segment %d: vaddr=0x%llx memsz=0x%llx filesz=0x%llx",
                 i, ph[i].p_vaddr, ph[i].p_memsz, ph[i].p_filesz);

        // You can derive RW from p_flags if you want; for now: RW+USER for simplicity
        uint64_t seg_flags = user_rw_flags;

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE)
        {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_critical("EXEC", "Out of pages while mapping ELF segment");
                return -1;
            }

            uint8_t *kva = (uint8_t*)(uintptr_t)pa;
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
                    (uint8_t*)data + ph[i].p_offset + offset_in_segment,
                    to_copy);
            }
        }

        /* Temporary safety net: one extra page after executable segment */
        if (ph[i].p_flags & PF_X) {
            uint64_t extra_va = seg_end;
            uint64_t extra_pa = pmm_alloc_page();
            if (extra_pa) {
                uint8_t *kva = (uint8_t*)(uintptr_t)extra_pa;
                memset(kva, 0, PAGE_SIZE);
                map_page(p->page_directory, extra_va, extra_pa, seg_flags);
            }
        }

    }

    // Set 64-bit entry point directly (already in high user range from linker)
    p->regs.rip = eh->e_entry;

    log_info("EXEC", "Process regs: RIP=0x%llx RSP=0x%llx",
            (unsigned long long)p->regs.rip,
            (unsigned long long)p->regs.rsp);

    uint64_t rip_pa = get_mapped_phys(p->page_directory, p->regs.rip);
    uint64_t rsp_pa = get_mapped_phys(p->page_directory, p->regs.rsp - 8);

    log_info("EXEC", "RIP VA=0x%llx -> PA=0x%llx",
            p->regs.rip, rip_pa);
    log_info("EXEC", "RSP VA=0x%llx -> PA=0x%llx",
            p->regs.rsp, rsp_pa);

    if (!rip_pa || !rsp_pa) {
        log_critical("EXEC", "ELF pages not mapped!");
    }

    log_info("EXEC", "ELF64 loaded entry=0x%llx", eh->e_entry);

    // debug_dump_user_bytes(p, p->regs.rip, 8);

    enter_user_mode_from_process(p);
    return 0;
}
