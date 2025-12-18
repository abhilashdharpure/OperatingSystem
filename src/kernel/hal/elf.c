#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "process.h"
#include "pmm.h"
#include "debug.h"
#include "paging.h"

#define ELF_MAGIC0 0x7F
#define ELF_MAGIC1 'E'
#define ELF_MAGIC2 'L'
#define ELF_MAGIC3 'F'
#define PAGE_SIZE 4096
// #define USER_STACK_TOP 0x80000000
#define USER_STACK_TOP 0x080ff000
// #define USER_STACK_SIZE 0x20000
#define USER_STACK_SIZE 0x10000  // 64 KB

uint32_t user_stack_top = 0xB0000000; // a high virtual address in user space

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

#define PT_LOAD 1

/* helpers you must have or implement */
extern uint32_t virt_to_phys(void *v);   /* returns physical address of kernel-virtual pointer */
extern void write_cr3(uint32_t pd_phys); /* wrapper assembly for mov cr3, ... */


// Return 0 if not mapped or on error. Otherwise physical address (pa = page_base | offset).
int phys_to_virt_ready = 0;

// Get the physical address mapped to a virtual address
uint32_t get_mapped_phys(uint32_t *pd_virt, uint32_t va) {
    uint32_t pd_idx = (va >> 22) & 0x3FF;
    uint32_t pt_idx = (va >> 12) & 0x3FF;
    uint32_t pde = pd_virt[pd_idx];

    // log_info("EXEC", "get_mapped_phys pde = %u, pd_idx = %u", pde, pd_idx);

    if (!(pde & PAGE_PRESENT))
    {
        log_info("EXEC", "get_mapped_phys if (!(pde & PAGE_PRESENT))");
        return 0;
    }

    uint32_t *pt_virt = pt_from_pde(pde);
    if (!pt_virt)
    {
        log_info("EXEC", "get_mapped_phys if (!pt_virt)");
        return 0;
    }

    uint32_t pte = pt_virt[pt_idx];
    if (!(pte & PAGE_PRESENT))
    {
        log_info("EXEC", "get_mapped_phys if (!(pte & PAGE_PRESENT))");
        return 0;
    }

    return pte & 0xFFFFF000;
}



void set_phys_to_virt_ready()
{
    phys_to_virt_ready = 1;
}

void dump_user_code(Process *p, uint32_t va, size_t len) {
    log_info("EXEC", "DUMP start 0x%x len=%u\n", va, (unsigned)len);
    for (size_t i = 0; i < len; ++i) {
        uint32_t pa = get_mapped_phys(p->page_directory, va + i);
        if (!pa) {
            log_info("EXEC", "DUMP: VA=0x%x not mapped\n", va + (unsigned)i);
            break;
        }
        uint8_t b = *(volatile uint8_t*)phys_to_virt(pa);
        if (i % 16 == 0) log_info("EXEC", "\n%08x: ", va + (unsigned)i);
        log_info("EXEC", "%u ", b);
    }
    log_info("EXEC", "\nDUMP end\n");
}


pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
{
    // log_info("EXEC", "exec_elf_mem start");

    Elf32_Ehdr *eh = (Elf32_Ehdr*)data;
    if (!eh->e_entry) {
        log_info("EXEC", "ELF has no entry");
        return -1;
    }

    // log_info("EXEC", "exec_elf_mem process_create");

    Process *p = process_create("user");
    if (!p) return -1;

    page_dir_t pd = create_user_pd();


    p->page_directory = pd.pd_virt;
    p->cr3 = pd.pd_phys;

    // in exec_elf_mem, right after create_user_pd:
    // log_info("PD", "PD phys = 0x%x virt = %p (stored in p: %p)", pd.pd_phys, pd.pd_virt, p->page_directory);

    // just before get_mapped_phys:
    // log_info("VERIFY", "Before get_mapped_phys: PDE[7]=0x%x, PDE[32]=0x%x", p->page_directory[7], p->page_directory[32]);

    // Clone kernel mappings so user can call kernel services
    clone_kernel_mappings(p->page_directory);


    // p->page_directory = pd;

    // uint32_t *pdv = p->page_directory;
    // pdv[7] = 0xDEADBEEF;
    // log_info("RAW", "Direct write PDE[7]=0x%x", p->page_directory[7]);

    // Find a suitable user memory region for stack
    MemoryRegion* user_region = NULL;
    for (int i = 0; i < bootParams->Memory.RegionCount; i++)
    {
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

    // log_info("STACK", "Selected User region =0x%x, length =0x%x", user_region->Begin, user_region->Length);

    // log_info("ELF", "Selected User Region: start=0x%llx length=0x%llx type=%x", user_region->Begin, user_region->Length, user_region->Type);

    uint32_t stack_top = 0x1FE0000;
    uint32_t stack_bottom = stack_top - USER_STACK_SIZE;

    // log_info("ELF", "Stack Top == 0x%x stack_bottom= 0x%x", stack_top, stack_bottom);

    for (uint32_t va = stack_bottom; va < stack_top; va += PAGE_SIZE)
    {
        uint32_t pa = pmm_alloc_page();
        if (!pa) {
            log_info("EXEC", "Out of pages for stack!");
            return -1;
        }

        memset((void*)pa, 0, PAGE_SIZE);  // identity for stack frames too
        // memset((void*)(pa + KERNEL_VMA), 0, PAGE_SIZE);

        // log_info("STACK", "Stack page: VA=0x%x -> PA=0x%x", va, pa);
        map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }
    p->regs.esp = stack_top;


    // log_info("ELF", "Before get_mapped_phys");

    uint32_t esp_pa_dbg = get_mapped_phys(p->page_directory, p->regs.esp - 4);
    // log_info("STACK", "After map: ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa_dbg);


    // Map ELF segments
    Elf32_Phdr *ph = (Elf32_Phdr*)((uint8_t*)data + eh->e_phoff);
    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;

        uint32_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
        uint32_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);


        for (uint32_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
            uint32_t pa = pmm_alloc_page();
            if (!pa) { /* handle error */ }

            // Identity or phys_to_virt(pa) depending on your choice; for Option 1:
            uint8_t *kva = (uint8_t*)pa;
            memset(kva, 0, PAGE_SIZE);

            map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);

            uint32_t offset_in_segment = va - seg_start;
            if (offset_in_segment < ph[i].p_filesz) {
                uint32_t to_copy = PAGE_SIZE;
                if (offset_in_segment + to_copy > ph[i].p_filesz)
                    to_copy = ph[i].p_filesz - offset_in_segment;

                memcpy(kva,
                    (uint8_t*)data + ph[i].p_offset + offset_in_segment,
                    to_copy);
            }
        }
    }

    // Set entry point
    p->regs.eip = eh->e_entry;

    // Verify mapping
    uint32_t eip_pa = get_mapped_phys(p->page_directory, p->regs.eip);
    uint32_t esp_pa = get_mapped_phys(p->page_directory, p->regs.esp - 4);

    // log_info("EXEC", "EIP VA=0x%x -> PA=0x%x", p->regs.eip, eip_pa);
    // log_info("EXEC", "ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa);
    if (!eip_pa || !esp_pa) {
        log_critical("EXEC", "ELF pages not mapped!");
    }

    // for (uint32_t va = 0x8047000; va < 0x8049000; va += 0x1000) {
    //     uint32_t pa = get_mapped_phys(p->page_directory, va);
    //     log_info("TEST", "VA=0x%x -> PA=0x%x", va, pa);
    // }


    log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

    // dump_user_code(p, 0x8048000, 0x60);

    // Switch to user mode
    enter_user_mode_from_process(p);

    return 0;
}

// pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
// {
//     log_info("EXEC", "exec_elf_mem start");

//     Elf32_Ehdr *eh = (Elf32_Ehdr*)data;
//     if (!eh->e_entry) {
//         log_info("EXEC", "ELF has no entry");
//         return -1;
//     }

//     Process *p = process_create("user");
//     if (!p) return -1;

//     // // Create new user page directory
//     // uint32_t *pd = create_user_pd();
//     // if (!pd) return -1;

//     page_dir_t pd = create_user_pd();
//     p->page_directory = pd.pd_virt;
//     p->cr3 = pd.pd_phys;

//     // log_info("PD", "PD phys = 0x%x virt = %p", pd.pd_phys, pd.pd_virt);

//     // in exec_elf_mem, right after create_user_pd:
//     log_info("PD", "PD phys = 0x%x virt = %p (stored in p: %p)", pd.pd_phys, pd.pd_virt, p->page_directory);

//     // just before get_mapped_phys:
//     log_info("VERIFY", "Before get_mapped_phys: PDE[7]=0x%x, PDE[32]=0x%x",
//             p->page_directory[7], p->page_directory[32]);


//     log_info("CHECK", "PDE[7]  = 0x%x", p->page_directory[7]);
//     log_info("CHECK", "PDE[32] = 0x%x", p->page_directory[32]);


//     // Clone kernel mappings so user can call kernel services
//     clone_kernel_mappings(p->page_directory);

//     // p->page_directory = pd;

//     uint32_t *pdv = p->page_directory;
//     pdv[7] = 0xDEADBEEF;
//     log_info("RAW", "Direct write PDE[7]=0x%x", p->page_directory[7]);

//     // Find a suitable user memory region for stack
//     MemoryRegion* user_region = NULL;
//     for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
//         if (bootParams->Memory.Regions[i].Type == 1 && 
//             bootParams->Memory.Regions[i].Begin >= 0x100000) {
//             user_region = &bootParams->Memory.Regions[i];
//             break;
//         }
//     }
//     if (!user_region) {
//         log_info("EXEC", "No user memory region available");
//         return -1;
//     }

//     // Compute stack top (page-aligned)
//     // uint32_t stack_top = (user_region->Begin + user_region->Length) & ~(PAGE_SIZE - 1);

//     // // Map stack downward
//     // for (uint32_t va = stack_top - USER_STACK_SIZE; va < stack_top; va += PAGE_SIZE) {
//     //     uint32_t pa = pmm_alloc_page();
//     //     if (!pa) {
//     //         log_info("EXEC", "Out of pages for stack");
//     //         return -1;
//     //     }
//     //     memset(phys_to_virt(pa), 0, PAGE_SIZE);
//     //     map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//     // }
//     // p->regs.esp = stack_top;

//     uint32_t stack_top = 0x1FE0000;
//     uint32_t stack_bottom = stack_top - USER_STACK_SIZE;

//     for (uint32_t va = stack_bottom; va < stack_top; va += 0x1000) {
//         uint32_t pa = pmm_alloc_page();
//         if (!pa) {
//             log_info("EXEC", "Out of pages for stack!");
//             return -1;
//         }
//         memset(phys_to_virt(pa), 0, 0x1000);
//         log_info("TEST", "PA allocated = 0x%x -> KV=0x%x", pa, phys_to_virt(pa));
//         map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_RW | PAGE_USER);
//     }
//     p->regs.esp = stack_top;


//     // Map ELF segments
//     // Elf32_Phdr *ph = (Elf32_Phdr*)((uint8_t*)data + eh->e_phoff);
//     // for (int i = 0; i < eh->e_phnum; i++) {
//     //     if (ph[i].p_type != PT_LOAD) continue;

//     //     uint32_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
//     //     uint32_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

//     //     log_info("EXEC", "Mapping ELF segment VA 0x%x - 0x%x", seg_start, seg_end);

//     //     // for (uint32_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
//     //     //     uint32_t pa = pmm_alloc_page();
//     //     //     if (!pa) {
//     //     //         log_info("EXEC", "Out of pages for ELF segment");
//     //     //         return -1;
//     //     //     }
//     //     //     void* kva = phys_to_virt(pa);
//     //     //     memset(kva, 0, PAGE_SIZE);
//     //     //     map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);

//     //     //     // Copy ELF file content (if inside file size)
//     //     //     uint32_t offset_in_segment = va - ph[i].p_vaddr;
//     //     //     if (offset_in_segment < ph[i].p_filesz) {
//     //     //         uint32_t to_copy = PAGE_SIZE;
//     //     //         if (offset_in_segment + to_copy > ph[i].p_filesz)
//     //     //             to_copy = ph[i].p_filesz - offset_in_segment;
//     //     //         memcpy(kva, (uint8_t*)data + ph[i].p_offset + offset_in_segment, to_copy);
//     //     //     }
//     //     // }

//     //     for (uint32_t va = seg_start; va < seg_end; va += 0x1000) {
//     //         uint32_t pa = pmm_alloc_page();
//     //         memset(phys_to_virt(pa), 0, 0x1000);
//     //         map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_RW | PAGE_USER);
//     //     }

//     // }


//     Elf32_Phdr *ph = (Elf32_Phdr*)((uint8_t*)data + eh->e_phoff);
//     for (int i = 0; i < eh->e_phnum; i++) {
//         if (ph[i].p_type != PT_LOAD) continue;

//         uint32_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
//         uint32_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

//         for (uint32_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
//             uint32_t pa = pmm_alloc_page();
//             if (!pa) { /* handle error */ }

//             // Identity or phys_to_virt(pa) depending on your choice; for Option 1:
//             uint8_t *kva = (uint8_t*)pa;
//             memset(kva, 0, PAGE_SIZE);

//             map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);

//             uint32_t offset_in_segment = va - seg_start;
//             if (offset_in_segment < ph[i].p_filesz) {
//                 uint32_t to_copy = PAGE_SIZE;
//                 if (offset_in_segment + to_copy > ph[i].p_filesz)
//                     to_copy = ph[i].p_filesz - offset_in_segment;

//                 memcpy(kva,
//                     (uint8_t*)data + ph[i].p_offset + offset_in_segment,
//                     to_copy);
//             }
//         }
//     }

//     // Set entry point
//     p->regs.eip = eh->e_entry;

//     // Verify mapping
//     uint32_t eip_pa = get_mapped_phys(p->page_directory, p->regs.eip);
//     uint32_t esp_pa = get_mapped_phys(p->page_directory, p->regs.esp - 4);

//     log_info("EXEC", "EIP VA=0x%x -> PA=0x%x", p->regs.eip, eip_pa);
//     log_info("EXEC", "ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa);
//     if (!eip_pa || !esp_pa) {
//         log_critical("EXEC", "ELF pages not mapped!");
//     }


//     for (uint32_t va = 0x8047000; va < 0x8049000; va += 0x1000) {
//         uint32_t pa = get_mapped_phys(p->page_directory, va);
//         log_info("TEST", "VA=0x%x -> PA=0x%x", va, pa);
//     }

//     // Switch to user mode
//     enter_user_mode_from_process(p);

//     return 0;
// }


// pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
// {
//     log_info("EXEC", "exec_elf_mem start");

//     Elf32_Ehdr *eh = (Elf32_Ehdr *)data;
//     if (!eh->e_entry) return -1;

//     Process *p = process_create("user");
//     if (!p) return -1;

//     uint32_t *pd = create_user_pd();
//     clone_kernel_mappings(pd);
//     p->page_directory = pd;

//     // Loop over all program headers
//     Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t*)data + eh->e_phoff);

//     // Choose highest usable region (from memory map)
//     MemoryRegion* user_region = NULL;
//     for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
//         if (bootParams->Memory.Regions[i].Type == 1 &&
//             bootParams->Memory.Regions[i].Begin >= 0x100000) {
//             user_region = &bootParams->Memory.Regions[i];
//             break;
//         }
//     }

//     if (!user_region)
//     {
//         log_info("EXEC", "exec_elf_mem if (!user_region)");
//         return -1;
//     }
        

//     uint32_t stack_top = (uint32_t)(user_region->Begin + user_region->Length);
//     stack_top &= ~(PAGE_SIZE - 1); // align down
//     p->regs.esp = stack_top;

//     // for (uint32_t va = USER_STACK_TOP - USER_STACK_SIZE; va < USER_STACK_TOP; va += PAGE_SIZE) {
//     //     uint32_t pa = pmm_alloc_page();
//     //     if (!pa) return -1;
//     //     memset(phys_to_virt(pa), 0, PAGE_SIZE);
//     //     map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//     // }
//     // uint32_t stack_top = (uint32_t)USER_STACK_TOP;
//     // p->regs.esp = USER_STACK_TOP;


//     // Map USER_STACK_SIZE downward
//     for (uint32_t va = stack_top - USER_STACK_SIZE; va < stack_top; va += PAGE_SIZE) {
//         uint32_t pa = pmm_alloc_page();
//         if (!pa) {
//             log_info("EXEC", "exec_elf_mem if (!pa)");
//             return -1;
//         }
//         memset(phys_to_virt(pa), 0, PAGE_SIZE);
//         map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//     }



//     // p->regs.esp = user_stack_top;
//     // for (uint32_t va = user_stack_top - USER_STACK_SIZE; va < user_stack_top; va += PAGE_SIZE) {
//     //     uint32_t pa = pmm_alloc_page();
//     //     memset(phys_to_virt(pa), 0, PAGE_SIZE);
//     //     map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//     // }



//     // // Allocate and map user stack
//     // uint32_t stack_pa = pmm_alloc_page();
//     // map_page(pd, USER_STACK_TOP - PAGE_SIZE, stack_pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//     // p->regs.esp = USER_STACK_TOP;


//     for (int i = 0; i < eh->e_phnum; i++) {
//         if (ph[i].p_type != PT_LOAD) continue;

//         uint32_t seg_start = ph[i].p_vaddr & ~(PAGE_SIZE - 1);
//         uint32_t seg_end   = (ph[i].p_vaddr + ph[i].p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

//         log_info("EXEC", "Mapping ELF segment: VA 0x%x - 0x%x", seg_start, seg_end);

//         for (uint32_t va = seg_start; va < seg_end; va += PAGE_SIZE) {
//             uint32_t pa = pmm_alloc_page();
//             void* kva = phys_to_virt(pa);
//             memset(kva, 0, PAGE_SIZE);
//             map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW);
//             log_info("EXEC", "Mapping VA=0x%x -> PA=0x%x", va, pa);


//             // Copy file data
//             uint32_t offset = va - ph[i].p_vaddr;
//             if (offset < ph[i].p_filesz) {
//                 uint32_t to_copy = PAGE_SIZE;
//                 if (offset + to_copy > ph[i].p_filesz)
//                     to_copy = ph[i].p_filesz - offset;
//                 memcpy(kva, (uint8_t*)data + ph[i].p_offset + offset, to_copy);
//             }
//         }
//     }

//     // Set entry
//     p->regs.eip = eh->e_entry;

//     // Now set phys_to_virt_ready for diagnostics
//     set_phys_to_virt_ready();

//     // Verify mappings
//     uint32_t eip_pa = get_mapped_phys(pd, p->regs.eip);
//     uint32_t esp_pa = get_mapped_phys(pd, p->regs.esp - 4);
//     log_info("EXEC", "EIP VA=0x%x -> PA=0x%x", p->regs.eip, eip_pa);
//     log_info("EXEC", "ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa);

//     enter_user_mode_from_process(p);
//     return 0;
// }


// pid_t exec_elf_mem(void *data, size_t size)
// {
//     Elf32_Ehdr *eh = (Elf32_Ehdr *)data;

//     if (eh->e_entry == 0) {
//         log_error("EXEC", "ELF entry is zero");
//         return -1;
//     }

//     Process *p = process_create("user");
//     if (!p) return -1;

//     uint32_t *pd = create_user_pd();
//     if (!pd) return -1;

//     clone_kernel_mappings(pd);
//     p->page_directory = pd;

//     Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t*)data + eh->e_phoff);

//     uint32_t segment_start = ph->p_vaddr & ~(PAGE_SIZE - 1);
//     uint32_t segment_end   = (ph->p_vaddr + ph->p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
//     uint32_t segment_offset = 0;

//     for (uint32_t va = segment_start; va < segment_end; va += PAGE_SIZE) {
//         uint32_t pa = pmm_alloc_page();
//         if (!pa) return -1;

//         memset(phys_to_virt(pa), 0, PAGE_SIZE);
//         if (map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW) != 0)
//             return -1;

//         // Copy portion of the segment into this page
//         uint32_t page_offset = 0;
//         uint32_t copy_start = va - ph->p_vaddr;       // offset in segment
//         uint32_t copy_size = PAGE_SIZE;
//         if (copy_start + copy_size > ph->p_filesz)
//             copy_size = ph->p_filesz - copy_start;
//         if (copy_size > 0) {
//             void* kva = phys_to_virt(pa);
//             memcpy(kva, (uint8_t*)data + ph->p_offset + copy_start, copy_size);
//         }
//     }

//     // #############################################################
//     p->regs.eip = eh->e_entry;
//     p->regs.esp = process_setup_stack(p);
//     log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

//     // #############################################################


//     // #######################  New code ######################################

//     // uint32_t stack_pa = pmm_alloc_page();
//     // map_page(pd, USER_STACK_TOP - USER_STACK_SIZE, stack_pa,
//     //         PAGE_PRESENT | PAGE_USER | PAGE_RW);

//     // p->regs.esp = USER_STACK_TOP;
//     // log_info("EXEC", "ELF loaded p->regs.esp =0x%x", p->regs.esp );
//     // log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

//     // p->regs.eip = eh->e_entry;

//     // #############################################################


//     // log_info("EXEC", "Calling get_mapped_phys");

//     // uint32_t eip_pa = get_mapped_phys(pd, p->regs.eip);
//     // log_info("EXEC", "EIP virtual=%p maps to phys=%p", p->regs.eip, eip_pa);
//     // if (!eip_pa) {
//     //     log_error("EXEC", "ERROR: entry point NOT mapped!");
//     //     return -1;
//     // }

//     // uint8_t *b = phys_to_virt(eip_pa);
//     // log_info("EXEC", "EIP first bytes: %02x %02x %02x %02x",
//     //         b[0], b[1], b[2], b[3]);


//     log_info("EXEC", "Checking EIP mapping...");
//     uint32_t eip_pa = get_mapped_phys(pd, p->regs.eip);
//     log_info("EXEC", "EIP VA=0x%x -> PA=0x%x", p->regs.eip, eip_pa);

//     if (eip_pa) {
//         uint8_t *b = phys_to_virt(eip_pa);
//         log_info("EXEC", "EIP bytes: %02x %02x %02x %02x", b[0], b[1], b[2], b[3]);
//     }

//     log_info("EXEC", "Checking ESP mapping...");
//     uint32_t esp_pa = get_mapped_phys(pd, p->regs.esp - 4);
//     log_info("EXEC", "ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa);


//     enter_user_mode_from_process(p);

//     return 0;
// }
