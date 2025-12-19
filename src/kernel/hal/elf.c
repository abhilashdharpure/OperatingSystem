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

#define USER_STACK_TOP 0x080ff000
#define USER_STACK_SIZE 0x10000  // 64 KB

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
extern uint32_t virt_to_phys(void *v);
extern void write_cr3(uint32_t pd_phys);

// Get the physical address mapped to a virtual address
uint32_t get_mapped_phys(uint32_t *pd_virt, uint32_t va)
{
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

pid_t exec_elf_mem(void *data, size_t size, BootParams* bootParams)
{
    Elf32_Ehdr *eh = (Elf32_Ehdr*)data;
    if (!eh->e_entry)
    {
        log_info("EXEC", "ELF has no entry");
        return -1;
    }

    Process *p = process_create("user");
    if (!p)
    {
        return -1;
    }

    page_dir_t pd = create_user_pd();

    p->page_directory = pd.pd_virt;
    p->cr3 = pd.pd_phys;

    // Clone kernel mappings so user can call kernel services
    clone_kernel_mappings(p->page_directory);

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

        // log_info("STACK", "Stack page: VA=0x%x -> PA=0x%x", va, pa);
        map_page(p->page_directory, va, pa, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }
    p->regs.esp = stack_top;

    uint32_t esp_pa_dbg = get_mapped_phys(p->page_directory, p->regs.esp - 4);
    // log_info("STACK", "After map: ESP VA=0x%x -> PA=0x%x", p->regs.esp, esp_pa_dbg);

    // Map ELF segments
    Elf32_Phdr *ph = (Elf32_Phdr*)((uint8_t*)data + eh->e_phoff);
    for (int i = 0; i < eh->e_phnum; i++)
    {
        if (ph[i].p_type != PT_LOAD)
        {
            continue;
        }

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
    if (!eip_pa || !esp_pa)
    {
        log_critical("EXEC", "ELF pages not mapped!");
    }

    log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

    // Switch to user mode
    enter_user_mode_from_process(p);

    return 0;
}