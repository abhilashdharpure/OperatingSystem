// paging.c – 64-bit paging for kernel + userspace

#include "paging.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <debug.h>
#include <arch/x86_64/cpu.h>
#include <hal/process.h>
#include <arch/x86_64/msr.h>


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
// mask physical address bits in a PTE (bits 12..51)
#define PTE_ADDR_MASK 0x000ffffffffff000ULL

extern uint64_t kernel_pml4_phys;
extern uint64_t *kernel_pml4_virt;
extern void write_cr3(uint64_t pa);
extern void enter_user_mode(Process *p);

// ----------------------------------------------------------------------
// Simple phys<->virt helpers for current IDENTITY mapping
// ----------------------------------------------------------------------

void dump_pte_for_va(uint64_t pml4_phys, uint64_t va)
{
    uint64_t *pml4 = (uint64_t *)phys_to_virt(pml4_phys);
    uint64_t idx_pml4 = (va >> 39) & 0x1FF;
    uint64_t idx_pdpt = (va >> 30) & 0x1FF;
    uint64_t idx_pd   = (va >> 21) & 0x1FF;
    uint64_t idx_pt   = (va >> 12) & 0x1FF;

    uint64_t pml4e = pml4[idx_pml4];
    log_info("PGDBG", "PML4[%llu]=0x%llx", idx_pml4, (unsigned long long)pml4e);
    if (!(pml4e & 1)) return;

    uint64_t pdpt_pa = pml4e & PTE_ADDR_MASK;
    uint64_t *pdpt = (uint64_t *)phys_to_virt(pdpt_pa);
    uint64_t pdpte = pdpt[idx_pdpt];
    log_info("PGDBG", " PDPT[%llu]=0x%llx", idx_pdpt, (unsigned long long)pdpte);
    if (!(pdpte & 1)) return;

    uint64_t pd_pa = pdpte & PTE_ADDR_MASK;
    uint64_t *pd = (uint64_t *)phys_to_virt(pd_pa);
    uint64_t pde = pd[idx_pd];
    log_info("PGDBG", "  PD[%llu]=0x%llx", idx_pd, (unsigned long long)pde);
    if (!(pde & 1)) return;

    uint64_t pt_pa = pde & PTE_ADDR_MASK;
    uint64_t *pt = (uint64_t *)phys_to_virt(pt_pa);
    uint64_t pte = pt[idx_pt];
    log_info("PGDBG", "   PT[%llu]=0x%llx", idx_pt, (unsigned long long)pte);

    log_info("PGDBG", "PT[%llu]=0x%llx (present=%d user=%d rw=%d nx=%d)",
         idx_pt, (unsigned long long)pte,
         !!(pte & PAGE_PRESENT), !!(pte & PAGE_USER), !!(pte & PAGE_RW), !!(pte & (1ULL<<63)));

}



void debug_dump_va_mapping(uint64_t *pml4, uint64_t va)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdpt_i = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    uint64_t pml4e = pml4[pml4_i];
    log_info("PGDBG", "VA=%llx PML4[%llu]=%llx", va, pml4_i, pml4e);
    if (!(pml4e & PAGE_PRESENT)) return;

    uint64_t *pdpt = (uint64_t*)phys_to_virt(pml4e & PTE_ADDR_MASK);
    uint64_t pdpte = pdpt[pdpt_i];
    log_info("PGDBG", "  PDPT[%llu]=%llx", pdpt_i, pdpte);
    if (!(pdpte & PAGE_PRESENT)) return;
    if (pdpte & PAGE_PS) {
        log_info("PGDBG", "  1GB page: base_pa=%llx", pdpte & ~0x3FFFFFFFULL);
        return;
    }

    uint64_t *pd   = (uint64_t*)phys_to_virt(pdpte & PTE_ADDR_MASK);
    uint64_t pde = pd[pd_i];
    log_info("PGDBG", "  PD[%llu]=%llx", pd_i, pde);
    if (!(pde & PAGE_PRESENT)) return;
    if (pde & PAGE_PS) {
        log_info("PGDBG", "  2MB page: base_pa=%llx", pde & ~0x1FFFFFULL);
        return;
    }

    uint64_t *pt   = (uint64_t*)phys_to_virt(pde & PTE_ADDR_MASK);
    uint64_t pte = pt[pt_i];
    log_info("PGDBG", "  PT[%llu]=%llx", pt_i, pte);
}


bool is_canonical(uint64_t va)
{
    uint64_t sign = (va >> 47) & 1ULL;

    if (sign == 0) {
        // upper bits must be 0
        return (va >> 48) == 0;
    } else {
        // upper bits must be all 1s
        return (va >> 48) == 0xFFFF;
    }
}


// ----------------------------------------------------------------------
// PML4 walking helpers
// ----------------------------------------------------------------------

// static uint64_t *get_or_alloc_pdp(uint64_t *pml4, uint64_t va, uint64_t flags)
// {
//     uint64_t idx = PML4_INDEX(va);
//     uint64_t e = pml4[idx];

//     if (!(e & PAGE_PRESENT)) {
//         uint64_t pa = pmm_alloc_page();
//         if (!pa) return NULL;

//         memset(phys_to_virt(pa), 0, PAGE_SIZE);
//         pml4[idx] = pa | flags | PAGE_PRESENT;
//         return (uint64_t *)phys_to_virt(pa);
//     } else {
//         uint64_t pa = e & ~0xFFFULL;
//         return (uint64_t *)phys_to_virt(pa);
//     }
// }

static uint64_t *get_or_alloc_pdp(uint64_t *pml4, uint64_t va, uint64_t flags)
{
    uint64_t idx = PML4_INDEX(va);
    uint64_t e = pml4[idx];

    if (!(e & PAGE_PRESENT)) {
        uint64_t pa = pmm_alloc_page();
        if (!pa)
        {
            log_info("Paging", "get_or_alloc_pdp: returning null for VA=0x%llx because pmm_alloc_page failed", va);
            return NULL;
        }
        memset(phys_to_virt(pa), 0, PAGE_SIZE);
        pml4[idx] = pa | flags | PAGE_PRESENT;
        return (uint64_t *)phys_to_virt(pa);
    }
    else 
    {
        uint64_t pa = e & PTE_ADDR_MASK;
        // Upgrade flags on existing entry (drop any high bits such as NX)
        uint64_t old_flags = e & 0xFFFULL;
        pml4[idx] = pa | old_flags | (flags & (PAGE_USER | PAGE_RW));
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

    uint64_t pa = e & PTE_ADDR_MASK;

    if (!(e & PAGE_PRESENT) || pa == 0) {
        uint64_t new_pa = pmm_alloc_page();
        if (!new_pa)
        {
            return NULL;
        }

        void *v = phys_to_virt(new_pa);
        memset(v, 0, PAGE_SIZE);
        pdp[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)phys_to_virt(new_pa);
    }
    else
    {
        /* Upgrade existing PD entry flags to include requested user/RW bits.
         * If the PD was copied from the kernel it may be supervisor-only,
         * which would block user access even if the PT entry is user-accessible.
         */
        uint64_t old_flags = e & 0xFFFULL;
        pdp[idx] = pa | old_flags | (flags & (PAGE_USER | PAGE_RW));
        return (uint64_t *)phys_to_virt(pa);
    }
}

static uint64_t *get_or_alloc_pt(uint64_t *pd, uint64_t va, uint64_t flags)
{
    uint64_t cr3 = read_cr3();
    uint64_t idx = PD_INDEX(va);
    uint64_t e   = pd[idx];
    uint64_t pa  = e & PTE_ADDR_MASK;

    if (!(e & PAGE_PRESENT) || pa == 0)
    {
        uint64_t new_pa = pmm_alloc_page();
        if (!new_pa)
        {
            return NULL;
        }

        void *v = phys_to_virt(new_pa);
        memset(v, 0, PAGE_SIZE);
        pd[idx] = new_pa | flags | PAGE_PRESENT;
        return (uint64_t *)v;
    }
    else
    {
        /* Upgrade existing PD entry flags to include user/RW as requested so
         * lower-level PT entries can be honored for user access.
         */
        uint64_t old_flags = e & 0xFFFULL;
        pd[idx] = pa | old_flags | (flags & (PAGE_USER | PAGE_RW));
        return (uint64_t *)phys_to_virt(pa);
    }
}

// Non-allocating lookup – used by get_mapped_phys
static uint64_t *get_pdp(uint64_t *pml4, uint64_t va)
{
    uint64_t e = pml4[PML4_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & PTE_ADDR_MASK);
}

static uint64_t *get_pd(uint64_t *pdp, uint64_t va)
{
    uint64_t e = pdp[PDP_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & PTE_ADDR_MASK);
}

static uint64_t *get_pt(uint64_t *pd, uint64_t va)
{
    uint64_t e = pd[PD_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return NULL;
    return (uint64_t *)phys_to_virt(e & PTE_ADDR_MASK);
}

int map_region(uint64_t *pml4, uint64_t start, uint64_t size, uint64_t flags)
{
    uint64_t end = start + size;
    for (uint64_t va = start; va < end; va += PAGE_SIZE) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            return -1;
        }
        memset(phys_to_virt(pa), 0, PAGE_SIZE);
        if (map_page(pml4, va, pa, flags) != 0) {
            return -1;
        }
    }
    return 0;
}


// ----------------------------------------------------------------------
// Low-level map / unmap / query
// ----------------------------------------------------------------------

int map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags)
{
    // log_info("MAP", "map_page: pml4=%p va=0x%llx pa=0x%llx flags=0x%llx", pml4, (unsigned long long)va, (unsigned long long)pa, (unsigned long long)flags);

    if (!pml4) {
        log_info("Paging", "map_page: pml4 pointer is NULL va=0x%llx pa=0x%llx flags=0x%llx",
                va, pa, flags);
        return -1;
    }

    // log_info("Paging", "map_page: pml4=%p va=0x%llx pa=0x%llx flags=0x%llx",
    //          pml4, va, pa, flags);
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
    // pt[idx] = (pa & ~0xFFFULL) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
    // pt[idx] = (pa & PTE_ADDR_MASK) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
    // pt[idx] = (pa & PTE_ADDR_MASK) | (flags & 0xFFFULL) | (flags & (1ULL<<63));

    pt[idx] = (pa & PTE_ADDR_MASK) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_NX));


    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");

    // log_info("MAP", "map_page: mapped va=0x%llx -> pa=0x%llx flags=0x%llx", (unsigned long long)va, (unsigned long long)pa, (unsigned long long)flags);

    return 0;
}

void unmap_page(uint64_t *pml4, uint64_t va)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdp_i  = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    if (!(pml4[pml4_i] & PAGE_PRESENT))
        return;

    uint64_t *pdp = phys_to_virt(pml4[pml4_i] & PTE_ADDR_MASK);
    if (!(pdp[pdp_i] & PAGE_PRESENT))
        return;

    uint64_t *pd = phys_to_virt(pdp[pdp_i] & PTE_ADDR_MASK);
    if (!(pd[pd_i] & PAGE_PRESENT))
        return;

    uint64_t *pt = phys_to_virt(pd[pd_i] & PTE_ADDR_MASK);
    if (!(pt[pt_i] & PAGE_PRESENT))
        return;

    pt[pt_i] = 0;

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
}

uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va)
{
    // log_info("PGDBG", "get_mapped_phys: pml4=%p va=0x%llx", pml4, va);
    uint64_t *pdp = get_pdp(pml4, va);
    if (!pdp) return 0;

    uint64_t *pd = get_pd(pdp, va);
    if (!pd) return 0;

    uint64_t *pt = get_pt(pd, va);
    if (!pt) return 0;

    uint64_t e = pt[PT_INDEX(va)];
    if (!(e & PAGE_PRESENT)) return 0;

    return (e & PTE_ADDR_MASK) | (va & 0xFFFULL);
}

int set_page_flags(uint64_t *pml4, uint64_t va, uint64_t flags)
{
    uint64_t pml4_i = PML4_INDEX(va);
    uint64_t pdp_i  = PDP_INDEX(va);
    uint64_t pd_i   = PD_INDEX(va);
    uint64_t pt_i   = PT_INDEX(va);

    if (!(pml4[pml4_i] & PAGE_PRESENT)) return -1;
    uint64_t pml4e_pa    = pml4[pml4_i] & PTE_ADDR_MASK;
    uint64_t pml4e_flags = pml4[pml4_i] & ~PTE_ADDR_MASK;
    pml4[pml4_i] = pml4e_pa | (pml4e_flags | (flags & (PAGE_USER | PAGE_RW)));

    uint64_t *pdp = phys_to_virt(pml4e_pa);
    if (!(pdp[pdp_i] & PAGE_PRESENT)) return -1;
    uint64_t pdpe_pa    = pdp[pdp_i] & PTE_ADDR_MASK;
    uint64_t pdpe_flags = pdp[pdp_i] & ~PTE_ADDR_MASK;
    pdp[pdp_i] = pdpe_pa | (pdpe_flags | (flags & (PAGE_USER | PAGE_RW)));

    uint64_t *pd = phys_to_virt(pdpe_pa);
    if (!(pd[pd_i] & PAGE_PRESENT)) return -1;
    uint64_t pde_pa    = pd[pd_i] & PTE_ADDR_MASK;
    uint64_t pde_flags = pd[pd_i] & ~PTE_ADDR_MASK;
    pd[pd_i] = pde_pa | (pde_flags | (flags & (PAGE_USER | PAGE_RW)));

    uint64_t *pt = phys_to_virt(pde_pa);
    if (!(pt[pt_i] & PAGE_PRESENT)) return -1;

    uint64_t pte_pa = pt[pt_i] & PTE_ADDR_MASK;
    pt[pt_i] = pte_pa | flags;

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
        uint64_t *pdp = get_pdp(pml4, va);
        if (!pdp) continue;

        uint64_t *pd = get_pd(pdp, va);
        if (!pd) continue;

        uint64_t *pt = get_pt(pd, va);
        if (!pt) continue;

        uint64_t idx = PT_INDEX(va);
        uint64_t e = pt[idx];
        if (!(e & PAGE_PRESENT))
            continue;

        uint64_t pa = e & PTE_ADDR_MASK;
        pt[idx] = pa | PAGE_PRESENT | PAGE_RW | PAGE_USER;

        __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    }
}

void clone_kernel_mappings_for_user(uint64_t *user_pml4)
{
    // PML4 already cloned from kernel CR3 in create_user_pd().
    // DO NOT clear PML4[0] — user mappings live there.

    // Just ensure user region is marked PAGE_USER.
    // make_user_mapping(user_pml4, USER_START, USER_END);

    // user_pml4[0] = 0;

    // Example: clear user bit on all entries except user range [0 .. USER_PML4_END)
    for (uint64_t i = USER_PML4_END; i < 512; i++) {
        uint64_t e = user_pml4[i];
        if (e & PAGE_PRESENT) {
            e &= ~PAGE_USER;
            user_pml4[i] = e;
        }
    }
    log_info("Paging",
             "clone_kernel_mappings_for_user: no extra changes");
}

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

    dump_pte_for_va(pml4_pa, p->regs.rip);
    dump_pte_for_va(pml4_pa, p->regs.rsp - 8);


    log_info("EXEC", "enter_user_mode: final RIP=0x%llx RSP=0x%llx CS=0x%llx SS=0x%llx",
         (unsigned long long)p->regs.rip,
         (unsigned long long)p->regs.rsp,
         (unsigned long long)p->regs.cs,
         (unsigned long long)p->regs.ss);
    debug_dump_va_mapping(p->page_directory, p->regs.rip);
    debug_dump_va_mapping(p->page_directory, p->regs.rsp);

    // before write_cr3 (already present) — add:
    log_info("EXEC", "enter_user_mode: p=%p cr3=0x%llx regs.rip=0x%llx regs.rsp=0x%llx cs=0x%llx ss=0x%llx rflags=0x%llx",
            p, (unsigned long long)pml4_pa,
            (unsigned long long)p->regs.rip,
            (unsigned long long)p->regs.rsp,
            (unsigned long long)p->regs.cs,
            (unsigned long long)p->regs.ss,
            (unsigned long long)p->regs.rflags);

    // dump PTE chain and first 64 bytes at RIP and RSP page-aligned
    dump_pte_for_va(pml4_pa, p->regs.rip);
    // debug_dump_user_bytes(p, p->regs.rip & ~(PAGE_SIZE-1), 64);
    dump_pte_for_va(pml4_pa, p->regs.rsp - 8);
    // debug_dump_user_bytes(p, p->regs.rsp & ~(PAGE_SIZE-1), 64);

    // print canonicality checks
    if ((p->regs.rsp & (1ULL<<63)) != 0) log_info("EXEC","RSP high bit set (non-canonical?) RSP=0x%llx", p->regs.rsp);
    if ((p->regs.rip & (1ULL<<63)) != 0) log_info("EXEC","RIP high bit set (non-canonical?) RIP=0x%llx", p->regs.rip);



    write_cr3(pml4_pa);
    log_info("EXEC", "enter_user_mode_from_process: after write_cr3");
    if (p->regs.rsp < USER_STACK_TOP - USER_STACK_SIZE || p->regs.rsp >= USER_STACK_TOP) {
        log_critical("EXEC", "RSP out of stack bounds: 0x%llx", p->regs.rsp);
    }


    // Restore FS/GS bases for user
    wrmsr(MSR_FS_BASE, p->fs_base ? p->fs_base : 0);
    wrmsr(MSR_GS_BASE, p->gs_base ? p->gs_base : 0);

    log_info("EXEC", "enter_user_mode_from_process: after write_msr, jumping to user");

    uint64_t cr3_now;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_now));
    log_info("EXEC", "enter_user_mode: CR3 about to switch to 0x%llx current_cr3=0x%llx", (unsigned long long)pml4_pa, (unsigned long long)cr3_now);

    if ((p->regs.rsp & (1ULL << 63)) != 0) {
        log_info("EXEC", "RSP looks canonical high bit set? RSP=0x%llx", (unsigned long long)p->regs.rsp);
    }

    log_info("EXEC", "Before jump to enter_user_mode");

    // debug_dump_user_bytes(p, p->regs.rsp, 128);

    if (!is_canonical(p->regs.rip) || !is_canonical(p->regs.rsp)) {
        log_critical("EXEC", "Non-canonical RIP/RSP!");
        for (;;);
    }

    log_info("EXEC", "dump_pte_for_va for start 0x7ffffe8a");

    dump_pte_for_va(p->cr3, 0x7ffffe8a);
    log_info("EXEC", "dump_pte_for_va for end 0x7ffffe8a");

    enter_user_mode(p);

    log_critical("EXEC", "enter_user_mode: returned unexpectedly from user mode");
    for (;;);
}
