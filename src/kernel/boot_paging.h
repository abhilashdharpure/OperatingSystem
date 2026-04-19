#pragma once

#include <stdint.h>
#include <boot/bootparams.h>   // for BootParams, MemoryRegion

#define PAGE_SIZE        4096
#define PAGE_PRESENT     (1ULL << 0)
#define PAGE_RW          (1ULL << 1)
#define PAGE_USER        (1ULL << 2)
#define PAGE_PS          (1ULL << 7)
#define PTE_ADDR_MASK    0x000ffffffffff000ULL

#define PML4_INDEX(va)   (((uint64_t)(va) >> 39) & 0x1FF)
#define PDP_INDEX(va)    (((uint64_t)(va) >> 30) & 0x1FF)
#define PD_INDEX(va)     (((uint64_t)(va) >> 21) & 0x1FF)
#define PT_INDEX(va)     (((uint64_t)(va) >> 12) & 0x1FF)

#define KERNEL_VMA_BASE  0xffffffff80000000ULL
#define KERNEL_LMA_BASE  0x0000000000200000ULL

#define BOOT_PMM_START_PA   0x00100000ULL   // 1 MiB
// #define BOOT_PMM_LIMIT_PA   0x01000000ULL   // 16 MiB
#define BOOT_PMM_LIMIT_PA 0x20000000ULL  // same 512 MiB

#define DIRECT_MAP_BASE   0xffff888000000000ULL

extern uint64_t boot_pdpt_identity[512];

void boot_pmm_init(void);
void switch_to_high_pml4(uint64_t *pml4);
void boot_setup_pml4(uint64_t *pml4);
int  boot_map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags);

/* NEW: pass BootParams so we can compute max_phys for direct map */
void setup_high_mappings(uint64_t *pml4, BootParams *bp);
uint64_t boot_get_boot_alloc_end(void);