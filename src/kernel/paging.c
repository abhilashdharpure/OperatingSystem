// paging.c – 64-bit paging for kernel + userspace

#include "paging.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <debug.h>
#include <arch/x86_64/cpu.h>
#include <hal/process.h>


#define USER_START 0x40000000ULL
#define USER_END   0x40200000ULL   // adjust to cover your program + stack



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

#define PAGE_PS (1ULL << 7)

extern uint64_t kernel_pml4_phys;
extern uint64_t *kernel_pml4_virt;
extern void write_cr3(uint64_t pa);
extern void enter_user_mode(Process *p);

// ----------------------------------------------------------------------
// Simple phys<->virt helpers for current IDENTITY mapping
// ----------------------------------------------------------------------



void debug_dump_va_mapping(uint64_t *pml4, uint64_t va)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdpt_i = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    uint64_t pml4e = pml4[pml4_i];
    log_info("PGDBG", "VA=%llx PML4[%llu]=%llx", va, pml4_i, pml4e);
    if (!(pml4e & PAGE_PRESENT)) return;

    uint64_t *pdpt = (uint64_t*)(pml4e & ~0xFFFULL);
    uint64_t pdpte = pdpt[pdpt_i];
    log_info("PGDBG", "  PDPT[%llu]=%llx", pdpt_i, pdpte);
    if (!(pdpte & PAGE_PRESENT)) return;
    if (pdpte & PAGE_PS) {
        log_info("PGDBG", "  1GB page: base_pa=%llx", pdpte & ~0x3FFFFFFFULL);
        return;
    }

    uint64_t *pd = (uint64_t*)(pdpte & ~0xFFFULL);
    uint64_t pde = pd[pd_i];
    log_info("PGDBG", "  PD[%llu]=%llx", pd_i, pde);
    if (!(pde & PAGE_PRESENT)) return;
    if (pde & PAGE_PS) {
        log_info("PGDBG", "  2MB page: base_pa=%llx", pde & ~0x1FFFFFULL);
        return;
    }

    uint64_t *pt = (uint64_t*)(pde & ~0xFFFULL);
    uint64_t pte = pt[pt_i];
    log_info("PGDBG", "  PT[%llu]=%llx", pt_i, pte);
}

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
        // log_info("Paging",
        //         "get_or_alloc_pd (alloc): idx=%llu va=0x%llx new_pa=0x%llx e=0x%llx",
        //         (unsigned long long)idx,
        //         (unsigned long long)va,
        //         (unsigned long long)new_pa,
        //         (unsigned long long)e);

        if (!new_pa)
        {
            return NULL;
        }

        void *v = phys_to_virt(new_pa);
        // log_info("Paging", "get_or_alloc_pd: phys_to_virt(new_pa)=%p", v);
        memset(v, 0, PAGE_SIZE);
        pdp[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)phys_to_virt(new_pa);
    }
    else
    {
        return (uint64_t *)phys_to_virt(pa);
    }
}

static uint64_t *get_or_alloc_pt(uint64_t *pd, uint64_t va, uint64_t flags)
{
    uint64_t cr3 = read_cr3();
    // log_info("Paging", "get_or_alloc_pt: CR3=0x%llx", (unsigned long long)cr3);

    uint64_t idx = PD_INDEX(va);
    uint64_t e   = pd[idx];
    uint64_t pa  = e & ~0xFFFULL;

    if (!(e & PAGE_PRESENT) || pa == 0)
    {
        uint64_t new_pa = pmm_alloc_page();
        // log_info("Paging",
        //          "get_or_alloc_pt (alloc): idx=%llu va=0x%llx new_pa=0x%llx e=0x%llx",
        //          (unsigned long long)idx,
        //          (unsigned long long)va,
        //          (unsigned long long)new_pa,
        //          (unsigned long long)e);

        if (!new_pa)
        {
            return NULL;
        }

        void *v = phys_to_virt(new_pa);
        // log_info("Paging", "get_or_alloc_pt: phys_to_virt(new_pa)=%p", v);
        memset(v, 0, PAGE_SIZE);
        pd[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)v;
    }
    else
    {
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
    if (!pdp)
    {
        log_critical("Paging", "map_page: get_or_alloc_pdp failed for VA=0x%llx", va);
        return -1;
    }

    uint64_t *pd = get_or_alloc_pd(pdp, va, flags);
    if (!pd)
    {
        log_critical("Paging", "map_page: get_or_alloc_pd failed for VA=0x%llx", va);
        return -1;
    }

    uint64_t *pt = get_or_alloc_pt(pd, va, flags);
    if (!pt)
    {
        log_critical("Paging", "map_page: get_or_alloc_pt failed for VA=0x%llx", va);
        return -1;
    }

    uint64_t idx = PT_INDEX(va);
    pt[idx] = (pa & ~0xFFFULL) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    return 0;
}

void unmap_page(uint64_t *pml4, uint64_t va)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdp_i  = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    uint64_t *pdp = (uint64_t *)(pml4[pml4_i] & ~0xFFFULL);
    if (!pdp) return;

    uint64_t *pd = (uint64_t *)(pdp[pdp_i] & ~0xFFFULL);
    if (!pd) return;

    uint64_t *pt = (uint64_t *)(pd[pd_i] & ~0xFFFULL);
    if (!pt) return;

    pt[pt_i] = 0;  // clear PTE

    // Optional: flush TLB for this VA
    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
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

int set_page_flags(uint64_t *pml4, uint64_t va, uint64_t flags)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdp_i  = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    uint64_t *pdp = (uint64_t *)(pml4[pml4_i] & ~0xFFFULL);
    if (!pdp || !(pml4[pml4_i] & PAGE_PRESENT)) return -1;

    uint64_t *pd = (uint64_t *)(pdp[pdp_i] & ~0xFFFULL);
    if (!pd || !(pdp[pdp_i] & PAGE_PRESENT)) return -1;

    uint64_t *pt = (uint64_t *)(pd[pd_i] & ~0xFFFULL);
    if (!pt || !(pd[pd_i] & PAGE_PRESENT)) return -1;

    if (!(pt[pt_i] & PAGE_PRESENT)) return -1;

    // Preserve physical address, change only flags:
    uint64_t pa = pt[pt_i] & ~0xFFFULL;
    pt[pt_i] = pa | flags;

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    return 0;
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


// recursively set PAGE_USER on all PT entries in a range
static void make_user_mapping(uint64_t *pml4, uint64_t start, uint64_t end)
{
    for (uint64_t va = start; va < end; va += PAGE_SIZE)
    {
        uint64_t *pdp = get_or_alloc_pdp(pml4, va, PAGE_PRESENT | PAGE_RW | PAGE_USER);
        if (!pdp) panic("make_user_mapping: get_or_alloc_pdp failed");

        uint64_t *pd = get_or_alloc_pd(pdp, va, PAGE_PRESENT | PAGE_RW | PAGE_USER);
        if (!pd) panic("make_user_mapping: get_or_alloc_pd failed");

        uint64_t *pt = get_or_alloc_pt(pd, va, PAGE_PRESENT | PAGE_RW | PAGE_USER);
        if (!pt) panic("make_user_mapping: get_or_alloc_pt failed");

        uint64_t idx = PT_INDEX(va);
        pt[idx] = (pt[idx] & ~0xFFFULL) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

        // Optional: invalidate TLB
        __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    }
}

// clone kernel PML4 + make user pages accessible
void clone_kernel_mappings_for_user(uint64_t *user_pml4)
{
    // Step 1: clone kernel PML4 entirely
    memcpy(user_pml4, kernel_pml4_virt, PAGE_SIZE);

    // Step 2: make PML4[0] point to a new PDPT (for user code) if needed
    uint64_t e0 = kernel_pml4_virt[0];
    if (e0 & PAGE_PRESENT)
    {
        uint64_t new_pdpt_pa = pmm_alloc_page();
        if (!new_pdpt_pa) panic("clone_kernel_mappings_for_user: failed to alloc PDPT");

        memset(phys_to_virt(new_pdpt_pa), 0, PAGE_SIZE);

        uint64_t *pdpt_src = (uint64_t *)phys_to_virt(e0 & ~0xFFFULL);
        uint64_t *pdpt_dst = (uint64_t *)phys_to_virt(new_pdpt_pa);

        for (int i = 0; i < 512; ++i)
            pdpt_dst[i] = pdpt_src[i];

        // Copy original flags but add PAGE_USER for PML4[0]
        user_pml4[0] = new_pdpt_pa | (e0 & 0xFFFULL) | PAGE_USER;
    }

    // Step 3: mark user code + stack region as USER
    make_user_mapping(user_pml4, USER_START, USER_END);

    log_info("Paging", "clone_kernel_mappings_for_user done: user VA 0x%llx-0x%llx now USER-accessible",
             USER_START, USER_END);
}


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
//         if (!new_pdpt_pa) panic("clone_kernel_mappings: failed to alloc PDPT");

//         memset(phys_to_virt(new_pdpt_pa), 0, PAGE_SIZE);

//         uint64_t *pdpt_src = (uint64_t *)phys_to_virt(e0 & ~0xFFFULL);
//         uint64_t *pdpt_dst = (uint64_t *)phys_to_virt(new_pdpt_pa);

//         for (int i = 0; i < 512; ++i) {
//             pdpt_dst[i] = pdpt_src[i];
//         }

//         // Copy original flags but ADD PAGE_USER so user can walk PML4[0]
//         user_pml4[0] = new_pdpt_pa | (e0 & 0xFFFULL) | PAGE_USER;
//     }
// }

// ----------------------------------------------------------------------
// Enter user mode (unchanged semantics – uses p->cr3 and enter_user_mode(p))
// ----------------------------------------------------------------------
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
