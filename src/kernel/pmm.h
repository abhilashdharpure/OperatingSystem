#pragma once
#include <stdint.h>
#include <boot/bootparams.h>

#define PAGE_SIZE 4096


// void pmm_free_page(uint32_t pa);
// uint32_t pmm_alloc_page(void);
// void pmm_init_from_regions(MemoryRegion *regions, int count);
// void pmm_init(void);



// void pmm_init(uint32_t memory_start);
void pmm_init(MemoryInfo* mem);

// void* pmm_alloc_page();
void pmm_mark_all_used(void);
int pmm_free_page(uintptr_t addr);
void pmm_reserve_region(uintptr_t base, uintptr_t size);
uint64_t pmm_alloc_page(void);