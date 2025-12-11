/* paging.c - simple i386 page directory + mapping helpers */
#include "paging.h"
#include <string.h>
#include <stdio.h> /* or your kernel log */
#include <arch/i686/irq.h>
#include <debug.h>


// extern uint32_t kernel_phys_offset; // usually 0xC0000000
// uint32_t kernel_phys_offset = 0xC0000000;
uint32_t *kernel_page_directory = 0;
uint32_t kernel_phys_offset = 0xC0000000;   // change if your linker uses another offset


extern uint32_t pmm_alloc_page(void);      /* must be provided by your PMM */
extern void phys_free_page(uint32_t pa);
extern void *phys_to_virt(uint32_t pa);     /* kernel virtual address for physical page */
extern uint32_t virt_to_phys(void *v);      /* optional */
extern void kpanic(const char *msg);        /* optional kernel panic */
extern void kprintf(const char *fmt, ...);  /* or log_info */

void write_cr3(uint32_t pa) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(pa) : "memory");
}

/* Allocate a zero'd physical page and return its PA,
 * or 0 on failure. This is wrapper around pmm_alloc_page.
 */
static uint32_t alloc_zeroed_page_phys(void) {
    uint32_t pa = pmm_alloc_page();
    if (!pa) return 0;
    void *kv = phys_to_virt(pa);
    memset(kv, 0, PAGE_SIZE);
    return pa;
}

/* Create a blank page directory (one physical page). Return kernel-virtual pointer
 * to the PD (so caller can write entries). The PD is a physical page, but we return
 * the kernel-virt mapping via phys_to_virt.
 */
uint32_t *create_page_directory(void) {
    uint32_t pd_pa = alloc_zeroed_page_phys();
    if (!pd_pa) return NULL;
    return (uint32_t*)phys_to_virt(pd_pa);
}

/* helper: get PDE pointer (kernel-virt) given pd_virt (kernel-virt pointer to PD) */
static inline uint32_t *pde_for(uint32_t *pd_virt, uint32_t va) {
    (void)pd_virt;
    uint32_t idx = (va >> 22) & 0x3FF;
    return &pd_virt[idx];
}

/* helper: given a PDE value return pointer to the page table (kernel-virt) or NULL */
static inline uint32_t *pt_from_pde(uint32_t pde) {
    if (!(pde & PAGE_PRESENT)) return NULL;
    uint32_t pt_pa = pde & 0xFFFFF000U;
    return (uint32_t*)phys_to_virt(pt_pa);
}

/* map_page: map single VA -> PA into PD given by pd_phys_ptr (kernel-virt pointer)
 * pd_phys_ptr: kernel-virt pointer to PD page (phys_to_virt(pd_pa))
 * returns 0 on success, -1 on failure
 */
int map_page(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa, uint32_t flags) {
    if (!pd_phys_ptr) return -1;
    uint32_t pd_idx = (va >> 22) & 0x3FF;
    uint32_t pt_idx = (va >> 12) & 0x3FF;

    uint32_t pde = pd_phys_ptr[pd_idx];
    uint32_t *pt_virt = NULL;

    if (!(pde & PAGE_PRESENT)) {
        /* allocate new page table */
        uint32_t new_pt_pa = alloc_zeroed_page_phys();
        if (!new_pt_pa) {
            log_info("paging", "map_page: alloc pt fail\n");
            return -1;
        }
        /* set PDE: base | present | rw | user (we'll set flags per mapping too) */
        pd_phys_ptr[pd_idx] = (new_pt_pa & 0xFFFFF000U) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        pt_virt = (uint32_t*)phys_to_virt(new_pt_pa);
    } else {
        pt_virt = pt_from_pde(pde);
        if (!pt_virt) return -1;
    }

    /* set PTE */
    uint32_t pte_val = (pa & 0xFFFFF000U) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
    pt_virt[pt_idx] = pte_val;

    /* Invalidate TLB for the virtual page */
    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    return 0;
}

/* map_region: map [va, va+len) to consecutive physical frames starting at pa_start
 * len must be multiple of PAGE_SIZE (caller responsibility if convenient)
 */
int map_region(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa_start, uint32_t len, uint32_t flags) {
    if (!pd_phys_ptr) return -1;
    if (va & (PAGE_SIZE - 1)) return -1; /* require page aligned VA */
    uint32_t pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t cur_va = va;
    uint32_t cur_pa = pa_start;
    for (uint32_t i = 0; i < pages; ++i) {
        if (map_page(pd_phys_ptr, cur_va, cur_pa, flags) != 0) {
            log_info("paging", "map_region: map_page fail at va=0x%08x\n", cur_va);
            return -1;
        }
        cur_va += PAGE_SIZE;
        cur_pa += PAGE_SIZE;
    }
    return 0;
}

/* Convenience: create a new user page directory and return kernel-virt pointer to PD page.
 * Caller must map kernel pages into this PD if they want kernel access after switching CR3.
 * For simplicity we will create a fresh PD that contains no kernel mappings.
 */
uint32_t *create_user_pd(void) {
    uint32_t *pd = create_page_directory();
    if (!pd) return NULL;
    /* Optionally copy some kernel mappings (like identity map low mem or kernel area).
     * For now keep PD minimal; user program must have all required mappings (code+stack).
     */
    return pd;
}

/* switch_page_dir: accept pd_virt (kernel-virt pointer to PD page) */
void switch_page_dir(uint32_t *pd_phys_ptr) {
    if (!pd_phys_ptr) return;
    /* get physical address: caller gave kernel-virt pointing into phys page; convert back */
    uint32_t pd_pa = 0;
#ifdef PHYS_FROM_VIRT /* if you have virt_to_phys, prefer that */
    pd_pa = virt_to_phys(pd_phys_ptr);
#else
    /* assume identity mapping (kernel-virtual == physical) */
    pd_pa = (uint32_t)pd_phys_ptr;
#endif
    write_cr3(pd_pa);
}

/* paging_map_user: convenience used by your earlier code.
 * This implementation will allocate physical pages and map them 1:1:
 * It maps [va, va+len) to newly allocated physical frames and returns 0 on success.
 */
int paging_map_user(uint32_t va, uint32_t len) {
    /* create a new PD for this process and store it somewhere global if you need to switch later.
       For quick testing we will create one and switch to it immediately.
     */
    uint32_t *pd = create_user_pd();
    if (!pd) { log_info("paging", "paging_map_user: create_user_pd failed\n"); return -1; }

    uint32_t pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t cur_va = va;
    for (uint32_t i = 0; i < pages; ++i) {
        uint32_t pa = pmm_alloc_page();
        if (!pa) { log_info("paging", "paging_map_user: pmm_alloc_page failed\n"); return -1; }
        void *kva = phys_to_virt(pa);
        memset(kva, 0, PAGE_SIZE); /* zero the new page */
        if (map_page(pd, cur_va, pa, PAGE_PRESENT | PAGE_RW | PAGE_USER) != 0) {
            log_info("paging", "paging_map_user: map_page failed for va=0x%08x\n", cur_va);
            return -1;
        }
        cur_va += PAGE_SIZE;
    }

    /* switch CR3 to this PD so future accesses use it */
    switch_page_dir(pd);
    return 0;
}

/* debug helper: print PDE/PTE presence/info */
void dump_pde_pte(uint32_t va) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    uint32_t *pd = (uint32_t*)phys_to_virt(cr3 & 0xFFFFF000U);
    uint32_t pd_idx = (va >> 22) & 0x3FF;
    uint32_t pde = pd[pd_idx];
    log_info("paging", "[dump_pde_pte] PDE[%u]=0x%08x\n", pd_idx, pde);
    if (!(pde & PAGE_PRESENT)) { log_info("paging", "[dump_pde_pte] PDE not present\n"); return; }
    uint32_t *pt = (uint32_t*)phys_to_virt(pde & 0xFFFFF000U);
    uint32_t pt_idx = (va >> 12) & 0x3FF;
    uint32_t pte = pt[pt_idx];
    log_info("paging", "[dump_pde_pte] PTE[%u]=0x%08x (P=%u U=%u W=%u)\n", pt_idx, pte, !!(pte & PAGE_PRESENT), !!(pte & PAGE_USER), !!(pte & PAGE_RW));
}
void clone_kernel_mappings(uint32_t *new_pd) {
    // extern uint32_t *kernel_page_directory;

    // Kernel identity-map is located in high half: PDEs 768–1023
    for (int i = 768; i < 1024; i++) {
        new_pd[i] = kernel_page_directory[i];
    }
}


// void* phys_to_virt(uint32_t pa) {
//     return (void*)(pa + kernel_phys_offset);
// }