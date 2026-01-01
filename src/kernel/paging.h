#pragma once
#include <stdint.h>
#include <stddef.h>
#include <hal/process.h>

// 64-bit page directory = PML4
typedef struct {
    uint64_t pd_phys;   // physical address of PML4
    uint64_t *pd_virt;  // kernel virtual address of PML4
} page_dir_t;

// Flags
#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_RW        (1ULL << 1)
#define PAGE_USER      (1ULL << 2)

void debug_dump_va_mapping(uint64_t *pml4, uint64_t va);

// Core mapping functions (64-bit)
int      map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags);
// void     unmap_page(uint64_t *pml4, uint64_t va);
uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va);
// int      map_region(uint64_t *pml4, uint64_t va, uint64_t pa_start, uint64_t len, uint64_t flags);

// User page table creation
page_dir_t create_user_pd(void);
void       clone_kernel_mappings(uint64_t *user_pml4);

// Enter user mode
void enter_user_mode_from_process(struct Process *p);