#include "pmm.h"
#include "debug.h"
#include "align.h"
#include <stdint.h>
#include <stddef.h>
#define MEM_USABLE   1
#define MEM_RESERVED 2
#define MEM_ACPI     3
#define MEM_NVS      4
#define MEM_BAD      5


#define PAGE_SIZE 4096
#define MEM_USABLE 1

typedef struct free_page {
    uint32_t next_pa; // physical address of next
} free_page_t;

static uint32_t free_list_head_pa = 0;

static uint32_t next_free_page = 0;
// #define PAGE_SIZE 4096
#define MAX_PAGES 262144 

extern char _kernel_start;
extern char _kernel_end;

static uintptr_t freelist[MAX_PAGES];
static size_t free_count = 0;

/* 512MB max = 131072 pages */
#define MAX_PAGES 131072
static uint8_t bitmap[MAX_PAGES / 8];

/* helpers */
static inline uint32_t align_up(uint32_t v)   { return (v + PAGE_SIZE - 1) & ~(PAGE_SIZE-1); }
static inline uint32_t align_down(uint32_t v) { return v & ~(PAGE_SIZE-1); }

// /* NOTE: you must have a function that converts PA -> kernel VA (phys_to_virt)
//    paging.c has a phys_to_virt() implementation which uses kernel_phys_offset.
//    We call phys_to_virt() here (declare it extern). */
// extern void *phys_to_virt(uint32_t pa);

// void pmm_free_page(uint32_t pa)
// {
//     if (pa == 0) return;
//     free_page_t *node = (free_page_t *)phys_to_virt(pa);
//     node->next_pa = free_list_head_pa;
//     free_list_head_pa = pa;
//     free_count++;
// }

// uint32_t pmm_alloc_page(void)
// {
//     if (!free_list_head_pa) {
//         log_error("PMM", "pmm_alloc_page: empty (free_count=%u)", (unsigned)free_count);
//         return 0;
//     }
//     uint32_t pa = free_list_head_pa;
//     free_page_t *node = (free_page_t *)phys_to_virt(pa);
//     free_list_head_pa = node->next_pa;
//     free_count--;
//     /* zero page in kernel virtual arena */
//     void *kva = phys_to_virt(pa);
//     memset(kva, 0, PAGE_SIZE);
//     return pa;
// }

// /* build free list from E820 style regions */
// void pmm_init_from_regions(MemoryRegion *regions, int count)
// {
//     free_list_head_pa = 0;
//     free_count = 0;

//     extern uintptr_t _kernel_start;
//     extern uintptr_t _kernel_end;
//     uint32_t kstart = align_up((uint32_t)(&_kernel_start));
//     uint32_t kend   = align_up((uint32_t)(&_kernel_end));

//     log_info("PMM", "kernel range PA: %08x - %08x", kstart, kend);

//     for (int i = 0; i < count; ++i) {
//         MemoryRegion *r = &regions[i];
//         if (r->Type != MEM_USABLE) continue;

//         uint32_t start = align_up((uint32_t)r->Begin);
//         uint32_t end = align_down((uint32_t)(r->Begin + r->Length));
//         if (end <= start) continue;

//         for (uint32_t pa = start; pa + PAGE_SIZE <= end; pa += PAGE_SIZE) {
//             if (pa >= kstart && pa < kend) continue; // skip kernel
//             pmm_free_page(pa);
//         }
//     }

//     log_info("PMM", "pmm_init: free_count=%u", (unsigned)free_count);
// }

// /* convenience entry: uses global boot params if present */
// extern BootParams *gBootParams;
// void pmm_init(void)
// {
//     if (gBootParams && gBootParams->Memory.RegionCount > 0) {
//         pmm_init_from_regions(gBootParams->Memory.Regions, gBootParams->Memory.RegionCount);
//     } else {
//         log_warning("PMM", "no boot regions; creating small emergency pool above kernel");
//         extern uintptr_t _kernel_end;
//         uint32_t start = align_up((uint32_t)(&_kernel_end));
//         uint32_t end   = start + (4 * 1024 * 1024); // 4MiB emergency pool
//         for (uint32_t pa = start; pa + PAGE_SIZE <= end; pa += PAGE_SIZE) pmm_free_page(pa);
//         log_info("PMM", "emergency pool free_count=%u", (unsigned)free_count);
//     }
// }



// void pmm_init(MemoryInfo* mem)
// {
//     next_free_page = _kernel_start;
// }


// uintptr_t pmm_alloc_page()
// {
//     uintptr_t out = next_free_page;
//     next_free_page += 4096;
//     return out;
// }


void pmm_init(MemoryInfo* mem)
{
    log_info("PMM", "Initializing Physical Memory Manager");

    // 1. Mark all pages as used
    pmm_mark_all_used();

    // 2. Free usable regions
    for (uint32_t i = 0; i < mem->RegionCount; i++) {
        MemoryRegion* r = &mem->Regions[i];
        log_info("PMM", "Initializing r->Type = %u", r->Type);

        if (r->Type != MEM_USABLE)
            continue;

        uint64_t start = ALIGN_UP(r->Begin, PAGE_SIZE);
        uint64_t end   = ALIGN_DOWN(r->Begin + r->Length, PAGE_SIZE);

        for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
            pmm_free_page(addr);
        }
    }

    // 3. Reserve kernel image
    // extern char _kernel_start[], _kernel_end[];
    pmm_reserve_region(_kernel_start, _kernel_end - _kernel_start);

    log_info("PMM", "PMM initialized");
}




void pmm_mark_all_used(void) {
    free_count = 0;
}

int pmm_free_page(uintptr_t phys) {
    if (free_count >= MAX_PAGES) {
        log_warning("PMM", "freelist full");
        return -1;
    }
    freelist[free_count++] = phys;
    return 0;
}

uintptr_t pmm_alloc_page() {
    if (free_count == 0) return NULL;
    uintptr_t a = freelist[--free_count];
    return (void*)a;
}

void pmm_reserve_region(uintptr_t start, uintptr_t length)
{
    // nothing to do for freelist-based PMM
}

// void pmm_reserve_region(uintptr_t start, uintptr_t length)
// {
//     uintptr_t addr = start & ~(PAGE_SIZE - 1);
//     uintptr_t end  = (start + length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

//     log_debug("PMM", "reserve region: 0x%x - 0x%x", addr, end);

//     for (; addr < end; addr += PAGE_SIZE)
//     {
//         pmm_mark_used((void *)addr);
//     }
// }





// static inline void set_bit(size_t bit) {
//     bitmap[bit / 8] |= (1 << (bit % 8));
// }
// static inline void clear_bit(size_t bit) {
//     bitmap[bit / 8] &= ~(1 << (bit % 8));
// }
// static inline int test_bit(size_t bit)
// {
//     return bitmap[bit / 8] & (1 << (bit % 8));
// }

// void pmm_mark_all_used(void) {
//     for (size_t i = 0; i < sizeof(bitmap); i++)
//         bitmap[i] = 0xFF;
// }

// void pmm_free_page(uintptr_t addr) {
//     size_t page = addr / PAGE_SIZE;
//     if (page < MAX_PAGES)
//         clear_bit(page);
// }

// void pmm_reserve_region(uintptr_t base, uintptr_t size) {
//     uintptr_t start = base / PAGE_SIZE;
//     uintptr_t pages = size / PAGE_SIZE;

//     for (uintptr_t i = 0; i < pages; i++)
//         set_bit(start + i);
// }

// uintptr_t pmm_alloc_page(void)
// {
//     // log_info("PMM", "pmm_alloc_page start");

//     for (size_t i = 0; i < MAX_PAGES; i++)
//     {
//         // log_info("PMM", "pmm_alloc_page i = %d", i);
        
//         if (!test_bit(i))
//         {
//             // log_info("PMM", "pmm_alloc_page if (!test_bit(i))");

//             set_bit(i);
//             return i * PAGE_SIZE;
//         }
//     }

//     // log_info("PMM", "pmm_alloc_page return 0");

//     return 0;
// }
