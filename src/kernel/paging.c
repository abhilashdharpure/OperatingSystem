// paging.c – 64-bit paging for kernel + userspace

#include "paging.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <debug.h>
#include <arch/x86_64/cpu.h>
#include <hal/process.h>

// --------- Assumptions / MUST align with your bootstrap + linker ----------
//
// 1) You are in long mode with 4-level paging.
// 2) The kernel is mapped in the higher half (canonical addresses >= 0xFFFF8000...),
//    and the current CR3 points to a PML4 that already has all kernel mappings set up.
// 3) Your bootstrap code exposes the current kernel PML4 like this:
//
//      extern uint64_t kernel_pml4_phys;
//      extern uint64_t *kernel_pml4_virt;
//
//    and ensures kernel_pml4_virt is a valid *kernel virtual* pointer to that PML4.
//
// 4) KERNEL_PML4_INDEX is the index in the PML4 where your kernel’s higher-half
//    mappings live. For a typical layout where kernel is at 0xFFFF800000000000,
//    that index is 256. Adjust if your linker script uses a different base.
// -------------------------------------------------------------------------

#define PAGE_SIZE            4096ULL

#define PML4_INDEX(va)       (((uint64_t)(va) >> 39) & 0x1FF)
#define PDP_INDEX(va)        (((uint64_t)(va) >> 30) & 0x1FF)
#define PD_INDEX(va)         (((uint64_t)(va) >> 21) & 0x1FF)
#define PT_INDEX(va)         (((uint64_t)(va) >> 12) & 0x1FF)

#define PAGE_PRESENT         (1ULL << 0)
#define PAGE_RW              (1ULL << 1)
#define PAGE_USER            (1ULL << 2)

// Adjust this to your actual kernel PML4 index (from linker/boot paging setup).
#define KERNEL_PML4_INDEX    256

// We allow userspace to use the lower half (0..255) PML4 entries.
#define USER_PML4_START      0
#define USER_PML4_END        KERNEL_PML4_INDEX

// Provided by your bootstrap (must be implemented there)
extern uint64_t kernel_pml4_phys;
extern uint64_t *kernel_pml4_virt;


// Simple phys<->virt helpers for kernel space
// If you already have better ones, hook them here.
static inline void *phys_to_virt(uint64_t pa)
{
    // If kernel is identity-mapped for low memory, this is fine.
    // Otherwise, add your kernel base offset here.
    return (void *)(uintptr_t)pa;
}

static inline uint64_t virt_to_phys(void *va)
{
    // Same caveat: adjust if kernel virtual != physical in low memory.
    return (uint64_t)(uintptr_t)va;
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
    uint64_t pa  = e & ~0xFFFULL;

    // Treat 'present with pa==0' as not-present/corrupt and allocate fresh
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
        return (uint64_t *)v;
    } else {
        log_info("Paging", "get_or_alloc_pd: else");
        // Existing valid PD
        return (uint64_t *)phys_to_virt(pa);
    }
}

static uint64_t *get_or_alloc_pt(uint64_t *pd, uint64_t va, uint64_t flags)
{
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

        void *v = phys_to_virt(new_pa);
        log_info("Paging", "get_or_alloc_pt: phys_to_virt(new_pa)=%p", v);
        memset(v, 0, PAGE_SIZE);
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
    log_info("Paging", "map_page: get_or_alloc_pd done");

    uint64_t *pt = get_or_alloc_pt(pd, va, flags);
    if (!pt) {
        log_critical("Paging", "map_page: get_or_alloc_pt failed for VA=0x%llx", va);
        return -1;
    }

    log_info("Paging", "map_page: get_or_alloc_pt done");


    uint64_t idx = PT_INDEX(va);
    pt[idx] = (pa & ~0xFFFULL) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
    log_info("Paging", "map_page: after pt[idx]  done");

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    log_info("Paging", "map_page: after invlpg");

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
// User page table creation (replaces old 32-bit create_user_pd)
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

    uint64_t cur_pml4_pa = read_cr3();
    uint64_t *cur_pml4   = (uint64_t *)phys_to_virt(cur_pml4_pa);

    // Clone the whole kernel address space
    memcpy(new_pml4, cur_pml4, PAGE_SIZE);

    log_info("Paging", "create_user_pd done: new_pml4_pa=0x%llx new_pml4=%p",
             new_pml4_pa, new_pml4);

    return (page_dir_t){
        .pd_phys = new_pml4_pa,
        .pd_virt = new_pml4
    };
}

void clone_kernel_mappings(uint64_t *user_pml4)
{
    (void)user_pml4;
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
            k_rsp, (uint64_t)p->regs.rip, pml4_pa);

    write_cr3(pml4_pa);

    log_info("EXEC", "enter_user_mode_from_process: after write_cr3, jumping to user");

    enter_user_mode(p);


    log_critical("EXEC", "enter_user_mode: returned unexpectedly from user mode");
    for (;;);
}


// ----------------------------------------------------------------------
// Clone kernel mappings (for compatibility with existing exec_elf_mem)
// ----------------------------------------------------------------------

// void clone_kernel_mappings(uint64_t *user_pml4)
// {
//     // For compatibility with your existing call in exec_elf_mem, we’ll ensure
//     // kernel higher-half entries are present. If create_user_pd already did this,
//     // this is effectively a no-op.
//     for (uint64_t i = KERNEL_PML4_INDEX; i < 512; i++) {
//         user_pml4[i] = kernel_pml4_virt[i];
//     }
// }



// // ----------------------------------------------------------------------
// // map_region – helper used by your ELF loader
// // ----------------------------------------------------------------------

// int map_region(uint64_t *pml4, uint64_t va, uint64_t pa_start, uint64_t len, uint64_t flags)
// {
//     if (!pml4) return -1;
//     if (va & (PAGE_SIZE - 1)) return -1;

//     uint64_t pages = (len + PAGE_SIZE - 1) / PAGE_SIZE;
//     uint64_t cur_va = va;
//     uint64_t cur_pa = pa_start;

//     for (uint64_t i = 0; i < pages; i++) {
//         if (map_page(pml4, cur_va, cur_pa, flags) != 0) {
//             log_error("Paging", "map_region: map_page failed at VA=0x%llx", cur_va);
//             return -1;
//         }
//         cur_va += PAGE_SIZE;
//         cur_pa += PAGE_SIZE;
//     }
//     return 0;
// }

