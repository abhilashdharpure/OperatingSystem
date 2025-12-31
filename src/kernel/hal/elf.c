#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "process.h"
#include "pmm.h"
#include "debug.h"
#include "paging.h"

// #define ELF_MAGIC0 0x7F
// #define ELF_MAGIC1 'E'
// #define ELF_MAGIC2 'L'
// #define ELF_MAGIC3 'F'
// #define PAGE_SIZE  4096

#define USER_STACK_TOP  0x080FF000U
#define USER_STACK_SIZE 0x00010000U  // 64 KB

#define PT_LOAD 1

// We now use get_mapped_phys from paging.c:
//   uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va);


pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
{
    (void)size;

    log_info("EXEC", "exec_elf_mem start");

    Elf64_Ehdr *eh = (Elf64_Ehdr*)data;

    // Basic sanity: 64-bit ELF, executable
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
    clone_kernel_mappings(p->page_directory);

    // Pick user memory region (unchanged)
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

    // Stack: keep using low canonical region for now
    uint64_t stack_top    = 0x0000000001FE0000ULL;
    uint64_t stack_bottom = stack_top - USER_STACK_SIZE;

    log_info("ELF", "Stack Top == 0x%llx stack_bottom=0x%llx",
             stack_top, stack_bottom);

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
                     PAGE_PRESENT | PAGE_RW | PAGE_USER) != 0)
        {
            log_critical("EXEC", "map_page failed for stack VA=0x%llx", va);
            return -1;
        }
    }

    p->regs.rsp = stack_top;   // 64-bit stack pointer

    uint64_t esp_pa_dbg = get_mapped_phys(p->page_directory, p->regs.rsp - 8);
    log_info("STACK", "After map: RSP VA=0x%llx -> PA=0x%llx",
             p->regs.rsp, esp_pa_dbg);

    // Map ELF64 segments
    Elf64_Phdr *ph = (Elf64_Phdr*)((uint8_t*)data + eh->e_phoff);
    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint64_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
        uint64_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        log_info("ELF", "Segment %d: vaddr=0x%llx memsz=0x%llx filesz=0x%llx",
                 i, ph[i].p_vaddr, ph[i].p_memsz, ph[i].p_filesz);

        for (uint64_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_critical("EXEC", "Out of pages while mapping ELF segment");
                return -1;
            }

            uint8_t *kva = (uint8_t*)(uintptr_t)pa;
            memset(kva, 0, PAGE_SIZE);

            if (map_page(p->page_directory,
                         va,
                         pa,
                         PAGE_PRESENT | PAGE_USER | PAGE_RW) != 0)
            {
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
    }

    // Set 64-bit entry point
    p->regs.rip = eh->e_entry;

    uint64_t rip_pa = get_mapped_phys(p->page_directory, p->regs.rip);
    uint64_t rsp_pa = get_mapped_phys(p->page_directory, p->regs.rsp - 8);

    log_info("EXEC", "RIP VA=0x%llx -> PA=0x%llx", p->regs.rip, rip_pa);
    log_info("EXEC", "RSP VA=0x%llx -> PA=0x%llx", p->regs.rsp, rsp_pa);
    if (!rip_pa || !rsp_pa) {
        log_critical("EXEC", "ELF pages not mapped!");
    }

    log_info("EXEC", "ELF64 loaded entry=0x%llx", eh->e_entry);

    enter_user_mode_from_process(p);
    return 0;
}



// pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
// {
//     (void)size;

//     log_info("EXEC", "exec_elf_mem start");

//     Elf32_Ehdr *eh = (Elf32_Ehdr*)data;
//     if (!eh->e_entry)
//     {
//         log_info("EXEC", "ELF has no entry");
//         return -1;
//     }

//     log_info("EXEC", "exec_elf_mem process_create");

//     Process *p = process_create("user");
//     if (!p)
//     {
//         return -1;
//     }

//     log_info("EXEC", "exec_elf_mem create_user_pd");

//     page_dir_t pd = create_user_pd();

//     // 64-bit: pd_virt is PML4 virtual, pd_phys is PML4 physical
//     p->page_directory = pd.pd_virt;
//     p->cr3            = pd.pd_phys;

//     log_info("EXEC", "exec_elf_mem clone_kernel_mappings");

//     // Ensure kernel higher-half mappings are present for this process
//     clone_kernel_mappings(p->page_directory);

//     // Find a suitable user memory region for stack (just for logging/debug)
//     MemoryRegion* user_region = NULL;
//     for (int i = 0; i < bootParams->Memory.RegionCount; i++)
//     {
//         if (bootParams->Memory.Regions[i].Type == 1 &&
//             bootParams->Memory.Regions[i].Begin >= 0x100000)
//         {
//             user_region = &bootParams->Memory.Regions[i];
//             break;
//         }
//     }
//     if (!user_region) {
//         log_info("EXEC", "No user memory region available");
//         return -1;
//     }

//     log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x",
//              user_region->Begin, user_region->Length, user_region->Type);

//     uint32_t stack_top    = 0x1FE0000U;
//     uint32_t stack_bottom = stack_top - USER_STACK_SIZE;

//     log_info("ELF", "Stack Top == 0x%x stack_bottom=0x%x",
//              stack_top, stack_bottom);

//     // Map user stack (low canonical virtual addresses, but 64-bit PML4)
//     for (uint32_t va = stack_bottom; va < stack_top; va += PAGE_SIZE)
//     {
//         uint64_t pa = pmm_alloc_page();
//         if (!pa) {
//             log_info("EXEC", "Out of pages for stack!");
//             return -1;
//         }

//         // Zero the physical page via its kernel VA
//         uint8_t *kva = (uint8_t*)(uintptr_t)pa;
//         memset(kva, 0, PAGE_SIZE);

//         // Map: VA (32-bit in low canonical region) -> PA (64-bit)
//         if (map_page(p->page_directory,
//                      (uint64_t)va,
//                      pa,
//                      PAGE_PRESENT | PAGE_RW | PAGE_USER) != 0)
//         {
//             log_critical("EXEC", "map_page failed for stack VA=0x%x", va);
//             return -1;
//         }
//     }
//     p->regs.esp = stack_top;

//     // Debug: verify stack mapping
//     uint64_t esp_pa_dbg = get_mapped_phys(p->page_directory, (uint64_t)(p->regs.esp - 4));
//     log_info("STACK", "After map: ESP VA=0x%x -> PA=0x%llx",
//              p->regs.esp, esp_pa_dbg);

//     // Map ELF segments
//     Elf32_Phdr *ph = (Elf32_Phdr*)((uint8_t*)data + eh->e_phoff);
//     for (int i = 0; i < eh->e_phnum; i++)
//     {
//         if (ph[i].p_type != PT_LOAD)
//             continue;

//         uint32_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
//         uint32_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

//         for (uint32_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
//             uint64_t pa = pmm_alloc_page();
//             if (!pa) {
//                 log_critical("EXEC", "Out of pages while mapping ELF segment");
//                 return -1;
//             }

//             uint8_t *kva = (uint8_t*)(uintptr_t)pa;
//             memset(kva, 0, PAGE_SIZE);

//             if (map_page(p->page_directory,
//                          (uint64_t)va,
//                          pa,
//                          PAGE_PRESENT | PAGE_USER | PAGE_RW) != 0)
//             {
//                 log_critical("EXEC", "map_page failed for ELF VA=0x%x", va);
//                 return -1;
//             }

//             uint32_t offset_in_segment = va - seg_start;
//             if (offset_in_segment < ph[i].p_filesz) {
//                 uint32_t to_copy = PAGE_SIZE;
//                 if (offset_in_segment + to_copy > ph[i].p_filesz)
//                     to_copy = ph[i].p_filesz - offset_in_segment;

//                 memcpy(kva,
//                        (uint8_t*)data + ph[i].p_offset + offset_in_segment,
//                        to_copy);
//             }
//         }
//     }

//     // Set entry point (still 32-bit VA in low canonical region)
//     p->regs.eip = eh->e_entry;

//     // Verify mapping of entrypoint and stack
//     uint64_t eip_pa = get_mapped_phys(p->page_directory, (uint64_t)p->regs.eip);
//     uint64_t esp_pa = get_mapped_phys(p->page_directory, (uint64_t)(p->regs.esp - 4));

//     log_info("EXEC", "EIP VA=0x%x -> PA=0x%llx", p->regs.eip, eip_pa);
//     log_info("EXEC", "ESP VA=0x%x -> PA=0x%llx", p->regs.esp, esp_pa);
//     if (!eip_pa || !esp_pa)
//     {
//         log_critical("EXEC", "ELF pages not mapped!");
//     }

//     log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

//     // Switch to user mode
//     enter_user_mode_from_process(p);

//     return 0;
// }
