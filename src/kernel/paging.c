// paging.c – 64-bit paging for kernel + userspace

#include "paging.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <debug.h>
#include <arch/x86_64/cpu.h>
#include <hal/process.h>

// --------- Current assumptions (IDENTITY kernel) -------------------------
//
// 1) You are in long mode with 4-level paging.
// 2) The kernel is currently mapped low, identity-mapped (VA == PA for low memory).
// 3) The bootstrap code set up a PML4 with a 1 GiB identity mapping for 0..1 GiB.
// 4) kernel_pml4_phys points to that PML4, and kernel_pml4_virt can be treated
//    as (uint64_t *)kernel_pml4_phys (identity mapping).
//
// Once you introduce a high-half physmap, you can switch phys_to_virt() to use
// PHYS_MAP_BASE again.
// -------------------------------------------------------------------------

#define PAGE_SIZE            4096ULL

#define PML4_INDEX(va)       (((uint64_t)(va) >> 39) & 0x1FF)
#define PDP_INDEX(va)        (((uint64_t)(va) >> 30) & 0x1FF)
#define PD_INDEX(va)         (((uint64_t)(va) >> 21) & 0x1FF)
#define PT_INDEX(va)         (((uint64_t)(va) >> 12) & 0x1FF)

#define PAGE_PRESENT         (1ULL << 0)
#define PAGE_RW              (1ULL << 1)
#define PAGE_USER            (1ULL << 2)

// Keep this for later when you move kernel to high-half and split user/kernel.
#define KERNEL_PML4_INDEX    256
#define USER_PML4_START      0
#define USER_PML4_END        KERNEL_PML4_INDEX

extern uint64_t kernel_pml4_phys;
extern uint64_t *kernel_pml4_virt;

// ----------------------------------------------------------------------
// Simple phys<->virt helpers for current IDENTITY mapping
// ----------------------------------------------------------------------

// For now, kernel low memory is identity-mapped: VA == PA for all paging
// structures and low RAM. This matches your current boot paging setup.
static inline void *phys_to_virt(uint64_t pa)
{
    return (void *)(uintptr_t)pa;
}

static inline uint64_t virt_to_phys(void *va)
{
    return (uint64_t)(uintptr_t)va;
}

// If you later introduce a physmap, you can switch to:
//
// #define PHYS_MAP_BASE  0xFFFF800000000000ULL
//
// static inline void* phys_to_virt(uint64_t pa)
// {
//     return (void *)(PHYS_MAP_BASE + pa);
// }
//
// static inline uint64_t virt_to_phys(void *va)
// {
//     return (uint64_t)((uintptr_t)va - PHYS_MAP_BASE);
// }

// ----------------------------------------------------------------------
// PML4 walking helpers
// ----------------------------------------------------------------------

static uint64_t *get_or_alloc_pdp(uint64_t *pml4, uint64_t va, uint64_t flags)
{
    uint64_t idx = PML4_INDEX(va);
    uint64_t e = pml4[idx];

    if (!(e & PAGE_PRESENT)) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) return NULL;

        memset(phys_to_virt(pa), 0, PAGE_SIZE);
        pml4[idx] = pa | flags | PAGE_PRESENT;
        return (uint64_t *)phys_to_virt(pa);
    } else {
        uint64_t pa = e & ~0xFFFULL;
        return (uint64_t *)phys_to_virt(pa);
    }
}

// static uint64_t *get_or_alloc_pd(uint64_t *pdp, uint64_t va, uint64_t flags)
// {
//     uint64_t idx = PDP_INDEX(va);
//     uint64_t e   = pdp[idx];
//     uint64_t pa  = e & ~0xFFFULL;

//     // Treat 'present with pa==0' as not-present/corrupt and allocate fresh
//     if (!(e & PAGE_PRESENT) || pa == 0) {
//         uint64_t new_pa = pmm_alloc_page();
//         log_info("Paging",
//                  "get_or_alloc_pd (alloc): idx=%llu va=0x%llx new_pa=0x%llx e=0x%llx",
//                  (unsigned long long)idx,
//                  (unsigned long long)va,
//                  (unsigned long long)new_pa,
//                  (unsigned long long)e);

//         if (!new_pa) return NULL;

//         void *v = phys_to_virt(new_pa);
//         log_info("Paging", "get_or_alloc_pd: phys_to_virt(new_pa)=%p", v);
//         memset(v, 0, PAGE_SIZE);
//         pdp[idx] = new_pa | flags | PAGE_PRESENT;
//         return (uint64_t *)v;
//     } else {
//         log_info("Paging", "get_or_alloc_pd: else");
//         // Existing valid PD
//         return (uint64_t *)phys_to_virt(pa);
//     }
// }
#define PAGE_PS (1ULL << 7)

static uint64_t *get_or_alloc_pd(uint64_t *pdp, uint64_t va, uint64_t flags)
{
    uint64_t idx = PDP_INDEX(va);
    uint64_t e   = pdp[idx];

    if (e & PAGE_PS) {
        log_critical("Paging",
                     "get_or_alloc_pd: cannot allocate PD under 1GiB huge page (idx=%llu e=0x%llx)",
                     (unsigned long long)idx,
                     (unsigned long long)e);
        panic("get_or_alloc_pd: 1GiB huge page in the way");
    }

    uint64_t pa = e & ~0xFFFULL;

    if (!(e & PAGE_PRESENT) || pa == 0) {
        uint64_t new_pa = pmm_alloc_page();
                log_info("Paging",
                 "get_or_alloc_pd (alloc): idx=%llu va=0x%llx new_pa=0x%llx e=0x%llx",
                 (unsigned long long)idx,
                 (unsigned long long)va,
                 (unsigned long long)new_pa,
                 (unsigned long long)e);

        if (!new_pa) return NULL;

        void *v = phys_to_virt(new_pa);
        log_info("Paging", "get_or_alloc_pd: phys_to_virt(new_pa)=%p", v);
        memset(v, 0, PAGE_SIZE);
        pdp[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)phys_to_virt(new_pa);
    } else {
        return (uint64_t *)phys_to_virt(pa);
    }
}


static uint64_t *get_or_alloc_pt(uint64_t *pd, uint64_t va, uint64_t flags)
{
    uint64_t cr3 = read_cr3();
    log_info("Paging", "get_or_alloc_pt: CR3=0x%llx", (unsigned long long)cr3);

    uint64_t idx = PD_INDEX(va);
    uint64_t e   = pd[idx];
    uint64_t pa  = e & ~0xFFFULL;

    if (!(e & PAGE_PRESENT) || pa == 0) {
        uint64_t new_pa = pmm_alloc_page();
        log_info("Paging",
                 "get_or_alloc_pt (alloc): idx=%llu va=0x%llx new_pa=0x%llx e=0x%llx",
                 (unsigned long long)idx,
                 (unsigned long long)va,
                 (unsigned long long)new_pa,
                 (unsigned long long)e);
        if (!new_pa) return NULL;

        log_info("Paging", "get_or_alloc_pt: before phys_to_virt");

        // void *v = phys_to_virt(new_pa);
        // log_info("Paging", "get_or_alloc_pt: phys_to_virt(new_pa)=%p", v);
        // memset(v, 0, PAGE_SIZE);
        // pd[idx] = new_pa | flags | PAGE_PRESENT;
        // return (uint64_t *)v;


        void *v = phys_to_virt(new_pa);
        log_info("Paging", "get_or_alloc_pt: phys_to_virt(new_pa)=%p", v);

        // DEBUG: check mapping of v in *kernel* CR3, not user_pml4
        extern uint64_t *kernel_pml4_virt;
        uint64_t mapped = get_mapped_phys(kernel_pml4_virt, (uint64_t)v);
        log_info("Paging", "KERNEL MAPCHK: VA=0x%llx -> PA=0x%llx",
                (unsigned long long)(uint64_t)v,
                (unsigned long long)mapped);

        // TEMPORARY: replace memset with manual zeroing
        uint8_t *p = (uint8_t *)v;
        log_info("Paging", "get_or_alloc_pt: 2");

        for (size_t i = 0; i < PAGE_SIZE; i++) {
            p[i] = 0;
        }
        log_info("Paging", "get_or_alloc_pt: 3");

        pd[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)v;

    } else {
        log_info("Paging", "get_or_alloc_pt: else");
        return (uint64_t *)phys_to_virt(pa);
    }
}

// Non-allocating lookup – used by get_mapped_phys
static uint64_t *get_pdp(uint64_t *pml4, uint64_t va)
{
    uint64_t e = pml4[PML4_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & ~0xFFFULL);
}

static uint64_t *get_pd(uint64_t *pdp, uint64_t va)
{
    uint64_t e = pdp[PDP_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & ~0xFFFULL);
}

static uint64_t *get_pt(uint64_t *pd, uint64_t va)
{
    uint64_t e = pd[PD_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & ~0xFFFULL);
}

// ----------------------------------------------------------------------
// Low-level map / unmap / query
// ----------------------------------------------------------------------

int map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags)
{
    uint64_t *pdp = get_or_alloc_pdp(pml4, va, flags);
    if (!pdp) {
        log_critical("Paging", "map_page: get_or_alloc_pdp failed for VA=0x%llx", va);
        return -1;
    }

    uint64_t *pd = get_or_alloc_pd(pdp, va, flags);
    if (!pd) {
        log_critical("Paging", "map_page: get_or_alloc_pd failed for VA=0x%llx", va);
        return -1;
    }
    // log_info("Paging", "map_page: get_or_alloc_pd done");

    uint64_t *pt = get_or_alloc_pt(pd, va, flags);
    if (!pt) {
        log_critical("Paging", "map_page: get_or_alloc_pt failed for VA=0x%llx", va);
        return -1;
    }

    // log_info("Paging", "map_page: get_or_alloc_pt done");

    uint64_t idx = PT_INDEX(va);
    pt[idx] = (pa & ~0xFFFULL) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
    // log_info("Paging", "map_page: after pt[idx] done");

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    // log_info("Paging", "map_page: after invlpg");

    return 0;
}

uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va)
{
    uint64_t *pdp = get_pdp(pml4, va);
    if (!pdp) return 0;

    uint64_t *pd = get_pd(pdp, va);
    if (!pd) return 0;

    uint64_t *pt = get_pt(pd, va);
    if (!pt) return 0;

    uint64_t e = pt[PT_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return 0;

    return (e & ~0xFFFULL) | (va & 0xFFFULL);
}

// ----------------------------------------------------------------------
// User page table creation
// ----------------------------------------------------------------------

page_dir_t create_user_pd(void)
{
    log_info("Paging", "create_user_pd (64-bit) start");

    uint64_t new_pml4_pa = pmm_alloc_page();
    if (!new_pml4_pa) {
        log_critical("Paging", "create_user_pd: no memory for PML4");
        return (page_dir_t){ .pd_phys = 0, .pd_virt = NULL };
    }

    uint64_t *new_pml4 = (uint64_t *)phys_to_virt(new_pml4_pa);

    uint64_t cur_pml4_pa = read_cr3() & ~0xFFFULL;
    uint64_t *cur_pml4   = (uint64_t *)phys_to_virt(cur_pml4_pa);

    // Clone the whole kernel address space (including identity mapping)
    memcpy(new_pml4, cur_pml4, PAGE_SIZE);

    log_info("Paging", "create_user_pd done: new_pml4_pa=0x%llx new_pml4=%p",
             (unsigned long long)new_pml4_pa, new_pml4);

    return (page_dir_t){
        .pd_phys = new_pml4_pa,
        .pd_virt = new_pml4
    };
}

// void clone_kernel_mappings(uint64_t *user_pml4)
// {
//     (void)user_pml4;
// }

// Rough sketch inside clone_kernel_mappings(pml4_user)
extern uint64_t *kernel_pml4_virt;

// void clone_kernel_mappings(uint64_t *user_pml4)
// {
//     for (int i = 0; i < 512; ++i) {
//         if (i == 0) {
//             user_pml4[i] = 0;   // no low-half identity for user
//         } else {
//             user_pml4[i] = kernel_pml4_virt[i];
//         }
//     }
// }
// void clone_kernel_mappings(uint64_t *user_pml4)
// {
//     // Copy top-level entries
//     for (int i = 0; i < 512; ++i) {
//         user_pml4[i] = kernel_pml4_virt[i];
//     }

//     // For index 0, allocate a new PDPT and copy contents,
//     uint64_t e0 = kernel_pml4_virt[0];
//     if (e0 & PAGE_PRESENT) {
//         uint64_t new_pdpt_pa = pmm_alloc_page();
//         memset(phys_to_virt(new_pdpt_pa), 0, PAGE_SIZE);

//         uint64_t *pdpt_src = (uint64_t *)phys_to_virt(e0 & ~0xFFFULL);
//         uint64_t *pdpt_dst = (uint64_t *)phys_to_virt(new_pdpt_pa);

//         for (int i = 0; i < 512; ++i) {
//             pdpt_dst[i] = pdpt_src[i];
//         }

//         user_pml4[0] = new_pdpt_pa | (e0 & 0xFFFULL);
//     }
// }
void clone_kernel_mappings(uint64_t *user_pml4)
{
    // Copy top-level entries
    for (int i = 0; i < 512; ++i) {
        user_pml4[i] = kernel_pml4_virt[i];
    }

    // For index 0, allocate a new PDPT and copy contents,
    uint64_t e0 = kernel_pml4_virt[0];
    if (e0 & PAGE_PRESENT) {
        uint64_t new_pdpt_pa = pmm_alloc_page();
        if (!new_pdpt_pa) panic("clone_kernel_mappings: failed to alloc PDPT");

        memset(phys_to_virt(new_pdpt_pa), 0, PAGE_SIZE);

        uint64_t *pdpt_src = (uint64_t *)phys_to_virt(e0 & ~0xFFFULL);
        uint64_t *pdpt_dst = (uint64_t *)phys_to_virt(new_pdpt_pa);

        for (int i = 0; i < 512; ++i) {
            pdpt_dst[i] = pdpt_src[i];
        }

        // Copy original flags but ADD PAGE_USER so user can walk PML4[0]
        user_pml4[0] = new_pdpt_pa | (e0 & 0xFFFULL) | PAGE_USER;
    }
}


// ----------------------------------------------------------------------
// Enter user mode (unchanged semantics – uses p->cr3 and enter_user_mode(p))
// ----------------------------------------------------------------------

extern void write_cr3(uint64_t pa);
extern void enter_user_mode(Process *p);

void enter_user_mode_from_process(Process *p)
{
    if (!p || !p->page_directory) {
        log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
        return;
    }

    uint64_t pml4_pa = p->cr3;
    if (!pml4_pa) {
        log_error("EXEC", "enter_user_mode: missing cr3");
        return;
    }

    uint64_t k_rsp;
    __asm__ volatile("mov %%rsp, %0" : "=r"(k_rsp));

    log_info("EXEC", "Before write_cr3: RSP=0x%llx, RIP(entry)=0x%llx, cr3=0x%llx",
             (unsigned long long)k_rsp,
             (unsigned long long)p->regs.rip,
             (unsigned long long)pml4_pa);

    write_cr3(pml4_pa);

    log_info("EXEC", "enter_user_mode_from_process: after write_cr3, jumping to user");

    enter_user_mode(p);

    log_critical("EXEC", "enter_user_mode: returned unexpectedly from user mode");
    for (;;);
}
