// arch/x86_64/paging_bootstrap.c (new file or existing one)
#include "paging_bootstrap.h"

#include <stdint.h>
#include "paging.h"
#include "debug.h"
#include "arch/x86_64/cpu.h"

uint64_t kernel_pml4_phys = 0;
uint64_t *kernel_pml4_virt = 0;

// If you already have a phys_to_virt(), use that instead.
static inline void *phys_to_virt_bootstrap(uint64_t pa)
{
    // If low memory is identity-mapped, this is fine for now.
    // Later you can adjust if the kernel lives fully in high-half.
    return (void *)(uintptr_t)pa;
}

void paging_init_long_mode_globals(void)
{
    kernel_pml4_phys = read_cr3() & ~0xFFFULL;
    kernel_pml4_virt = (uint64_t *)phys_to_virt_bootstrap(kernel_pml4_phys);

    log_info("Paging", "kernel_pml4_phys=0x%llx kernel_pml4_virt=%p",
             kernel_pml4_phys, kernel_pml4_virt);


    for (uint64_t pa = 0x00100000; pa < 0x02000000; pa += 0x00100000) {
        uint64_t va = pa;  // identity
        uint64_t mapped = get_mapped_phys(kernel_pml4_virt, va);
        log_info("MAPCHK", "VA=0x%llx -> PA=0x%llx", va, mapped);
    }

}
