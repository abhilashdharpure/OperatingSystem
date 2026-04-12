#pragma once

#include <stdint.h>


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
#define KERNEL_LMA_BASE 0x0000000000200000ULL

#define BOOT_PMM_START_PA   0x00100000ULL   // 1 MiB
#define BOOT_PMM_LIMIT_PA   0x01000000ULL   // 16 MiB



/*
 * Boot‑only paging API.
 * These functions are used exclusively inside the low stub
 * before switch_to_high_pml4() is executed.
 *
 * All implementations live in boot_paging.c and are placed
 * in .boot64_stub / .boot64_stub_data.
 */

extern uint64_t boot_pdpt_identity[512];

void boot_pmm_init(void);

void switch_to_high_pml4(uint64_t *pml4);

/* Map the kernel into higher‑half and install identity mapping */
void setup_high_mappings(uint64_t *pml4);

/* Map a single page (boot‑only version, no logging, no full PMM) */
int boot_map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags);

/* Initialize PML4 with identity mapping for low memory */
void boot_setup_pml4(uint64_t *pml4);

/* Tiny boot allocator: returns a physical page (identity‑mapped) */
uint64_t boot_alloc_page(void);

