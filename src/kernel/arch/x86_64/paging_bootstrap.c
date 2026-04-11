// arch/x86_64/paging_bootstrap.c (new file or existing one)
#include "paging_bootstrap.h"

#include <stdint.h>
#include "paging.h"
#include "debug.h"
#include "arch/x86_64/cpu.h"

uint64_t kernel_pml4_phys = 0;
uint64_t *kernel_pml4_virt = 0;

void paging_init_long_mode_globals(void)
{
    kernel_pml4_phys = read_cr3() & ~0xFFFULL;
    kernel_pml4_virt = (uint64_t *)phys_to_virt(kernel_pml4_phys);

    log_info("Paging", "kernel_pml4_phys=0x%llx kernel_pml4_virt=%p",
             kernel_pml4_phys, kernel_pml4_virt);

    // Dump PML4[0]
    uint64_t e0 = kernel_pml4_virt[0];
    log_info("Paging", "PML4[0]=0x%llx", (unsigned long long)e0);

    if (e0 & 1) {
        uint64_t pdpt_pa = e0 & ~0xFFFULL;
        uint64_t *pdpt = (uint64_t *)phys_to_virt(pdpt_pa);
        uint64_t pdpte0 = pdpt[0];
        log_info("Paging", "PDPT[0]=0x%llx", (unsigned long long)pdpte0);
    }
}

