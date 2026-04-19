#include "pmm.h"
#include "debug.h"
#include "align.h"
#include "paging.h"
#include <stdint.h>
#include <stddef.h>
#define MEM_USABLE   1
#define MEM_RESERVED 2
#define MEM_ACPI     3
#define MEM_NVS      4
#define MEM_BAD      5


#define PAGE_SIZE        4096
#define PMM_FREE_START   0x00100000ULL      // 1 MiB
#define PMM_MAX_IDENTITY 0x20000000ULL      // 512 MiB

#define KERNEL_VMA 0xffffffff80000000ULL

typedef struct free_page {
    uint32_t next_pa; // physical address of next
} free_page_t;

static uint32_t free_list_head_pa = 0;

static uint32_t next_free_page = 0;
#define MAX_PAGES 262144 

extern char _kernel_start;
extern char _kernel_end;


extern char _kernel_phys_start;
extern char _kernel_phys_end;

static uintptr_t freelist[MAX_PAGES];
static size_t free_count = 0;

/* 512MB max = 131072 pages */
#define MAX_PAGES 131072
static uint8_t bitmap[MAX_PAGES / 8];

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
    free_count = 0;

    /* 1) Build freelist from usable regions in [PMM_FREE_START, PMM_MAX_IDENTITY) */
    for (uint32_t i = 0; i < mem->RegionCount; i++) {
        MemoryRegion *r = &mem->Regions[i];
        if (r->Type != MEM_USABLE)
            continue;

        uintptr_t start = align_up(r->Begin);
        uintptr_t end   = align_down(r->Begin + r->Length);

        if (start < PMM_FREE_START)
            start = PMM_FREE_START;
        if (end > PMM_MAX_IDENTITY)
            end = PMM_MAX_IDENTITY;

        if (start >= end)
            continue;

        for (uintptr_t pa = start; pa < end; pa += PAGE_SIZE)
            pmm_free_page(pa);
    }

    /* 2) Reserve all pages used by the boot stub allocator (page tables, etc.) */
    uintptr_t boot_end = boot_get_boot_alloc_end();
    if (boot_end > PMM_FREE_START) {
        pmm_reserve_region(PMM_FREE_START, boot_end - PMM_FREE_START);
    }

    /* 3) Compute kernel physical range from higher-half symbols */
    uintptr_t kernel_phys_start = kernel_virt_to_phys(&_kernel_start);
    uintptr_t kernel_phys_end   = kernel_virt_to_phys(&_kernel_end);

    log_info("PMM", "kernel_phys: [0x%llx, 0x%llx)",
             (unsigned long long)kernel_phys_start,
             (unsigned long long)kernel_phys_end);

    /* 4) Reserve the kernel image itself */
    pmm_reserve_region(kernel_phys_start,
                       kernel_phys_end - kernel_phys_start);

    log_info("PMM", "Reserving kernel region: [0x%llx, 0x%llx)",
             (unsigned long long)kernel_phys_start,
             (unsigned long long)kernel_phys_end);

    /* 5) Debug + sanity check */
    log_info("PMM", "free_count=%u pages (~%u KiB)",
             (unsigned)free_count,
             (unsigned)(free_count * 4));

    for (size_t i = 0; i < free_count; i++) {
        if (freelist[i] < PMM_FREE_START ||
            freelist[i] >= PMM_MAX_IDENTITY)
            panic("PMM: freelist contains invalid entry");
    }
}

void pmm_mark_all_used(void)
{
    free_count = 0;
}

int pmm_free_page(uintptr_t phys)
{
    if ((phys & (PAGE_SIZE - 1)) != 0 ||
        phys < PMM_FREE_START ||
        phys >= PMM_MAX_IDENTITY)
    {
        log_critical("PMM", "pmm_free_page: BAD phys=0x%llx", (unsigned long long)phys);
        panic("pmm_free_page: invalid phys");
    }

    if (free_count >= MAX_PAGES) {
        panic("pmm_free_page: freelist overflow");
    }

    freelist[free_count++] = phys;
    return 0;
}

uint64_t pmm_alloc_page(void)
{
    if (free_count == 0) {
        log_info("PMM", "out of pages!");
        return 0;
    }

    uintptr_t pa = freelist[free_count - 1];

    if ((pa & (PAGE_SIZE - 1)) != 0 ||
        pa < PMM_FREE_START ||
        pa >= PMM_MAX_IDENTITY)
    {
        log_critical("PMM",
            "CORRUPT freelist[%u] = 0x%x (free_count=%u)",
            (unsigned)(free_count - 1),
            (uint32_t)pa,
            (unsigned)free_count);

        // dump a window
        for (int i = -4; i <= 4; i++)
        {
            int idx = (int)free_count - 1 + i;
            if (idx < 0 || (size_t)idx >= free_count) continue;
            log_critical("PMM",
                "  freelist[%d] = 0x%x",
                idx,
                (uint32_t)freelist[idx]);
        }

        panic("PMM freelist corrupted");
    }

    free_count--;
    return (uint64_t)pa;
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
