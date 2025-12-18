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
// #define MEM_USABLE 1
/* kernel is mapped at 0xC0000000 */
#define KERNEL_VIRT_OFFSET 0xC0000000

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
// static inline uint32_t align_up(uint32_t v)   { return (v + PAGE_SIZE - 1) & ~(PAGE_SIZE-1); }
// static inline uint32_t align_down(uint32_t v) { return v & ~(PAGE_SIZE-1); }

/* helpers */
static inline uintptr_t align_up(uintptr_t v)
{
    return (v + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

static inline uintptr_t align_down(uintptr_t v) 
{
    return v & ~(PAGE_SIZE - 1);
}

void pmm_init(MemoryInfo *mem)
{
    pmm_mark_all_used();

    // log_info("PMM", "mem->RegionCount = %u", mem->RegionCount);

    /* 1) Free all usable RAM */
    for (uint32_t i = 0; i < mem->RegionCount; i++)
    {
        MemoryRegion *r = &mem->Regions[i];

        if (r->Type != MEM_USABLE)
        {
            continue;
        }

        uintptr_t start = align_up(r->Begin);
        uintptr_t end   = align_down(r->Begin + r->Length);
        // log_info("PMM", "Stack page: start = %u, end = %u", start, end);

        for (uintptr_t pa = start; pa < end; pa += PAGE_SIZE)
        {
            pmm_free_page(pa);
        }
    }

    /* 2) Reserve first 1MB (BIOS, DMA, bootloader) */
    pmm_reserve_region(0, 0x100000);

    /* 3) Reserve kernel image (convert VA → PA!) */
    uintptr_t kernel_phys_start =
        (uintptr_t)&_kernel_start - KERNEL_VIRT_OFFSET;
    uintptr_t kernel_phys_end =
        (uintptr_t)&_kernel_end - KERNEL_VIRT_OFFSET;

    pmm_reserve_region(kernel_phys_start,
                       kernel_phys_end - kernel_phys_start);

    // log_info("PMM", "Initialized, free pages=%u", free_count);
}

void pmm_mark_all_used(void) {
    free_count = 0;
}

int pmm_free_page(uintptr_t phys)
{
    if (free_count >= MAX_PAGES)
        return -1;

    // log_info("PMM", "PMM Stack[%d] = 0x%x", free_count, phys);

    freelist[free_count++] = phys;
    return 0;
}

uintptr_t pmm_alloc_page(void)
{
    if (free_count == 0)
    {
        log_info("PMM", "out of pages!");
        return 0;
    }

    return freelist[--free_count];
}

void pmm_reserve_region(uintptr_t start, uintptr_t length)
{
    uintptr_t end = start + length;

    for (size_t i = 0; i < free_count; i++) {
        uintptr_t pa = freelist[i];

        if (pa >= start && pa < end) {
            freelist[i] = freelist[--free_count];
            i--;
        }
    }
}

