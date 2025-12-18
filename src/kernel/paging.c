/* paging.c - simple i386 page directory + mapping helpers */
#include "paging.h"
#include <string.h>
#include <stdio.h> /* or your kernel log */
#include <arch/i686/irq.h>
#include <debug.h>
#include "hal/process.h"

#include <stdint.h>
#include <stdbool.h>

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4
#define PAGE_SIZE    4096

// High-half kernel base
#define KERNEL_VMA   0xC0000000
// #define TEMP_VMA 0xC1000000
// #define TEMP_VMA 0xFFC00000  // Temporary virtual address for mapping
#define TEMP_VMA       0xFFC01000   // temporary mapping for new user PD
#define TEMP_KPD_VMA   0xFFC02000   // temporary mapping for kernel PD access


#define KERNEL_PD_IDX   (KERNEL_VMA >> 22)

// Physical addresses for initial PD/PT
// #define KERNEL_PD_PHYS 0x001F4000
#define KERNEL_PD_PHYS (kernel_page_directory_phys)

#define KERNEL_PT_PHYS 0x001F5000

// extern char _kernel_start, _kernel_end;
extern char _kernel_start[];
extern char _kernel_end[];
extern char _kernel_stack_bottom[];
extern char _kernel_stack_top[];

// Globals
uint32_t kernel_page_directory_phys;
uint32_t *kernel_page_directory;

// Kernel stack
#define KERNEL_STACK_SIZE (16*1024)
#define KERNEL_STACK_TOP  (0x00800000) // 8MB top

#define PHYS_FROM_VIRT 1

uint32_t *kernel_page_directory = 0;
uint32_t kernel_phys_offset = 0xC0000000;   // change if your linker uses another offset


extern uint32_t pmm_alloc_page(void);      /* must be provided by your PMM */
extern void phys_free_page(uint32_t pa);
extern void *phys_to_virt(uint32_t pa);     /* kernel virtual address for physical page */
extern uint32_t virt_to_phys(void *v);      /* optional */
extern void kpanic(const char *msg);        /* optional kernel panic */
extern void kprintf(const char *fmt, ...);  /* or log_info */

void write_cr3(uint32_t pa)
{
    __asm__ volatile("mov %0, %%cr3" :: "r"(pa) : "memory");

    log_info("VERIFY", "write_cr3 pa=0x%x", pa);

}

void map_identity_page(uint32_t* pd, uintptr_t pa)
{
    uint32_t pd_idx = (pa >> 22) & 0x3FF;
    uint32_t pt_idx = (pa >> 12) & 0x3FF;

    uint32_t* pt;

    if (!(pd[pd_idx] & 0x1)) // If page table not present
    {
        pt = (uint32_t*)pmm_alloc_page();  // Allocate a page for page table
        for (int i = 0; i < 1024; i++)
            pt[i] = 0;

        pd[pd_idx] = ((uintptr_t)pt) | 0x3; // Present + RW
    }
    else
    {
        pt = (uint32_t*)(pd[pd_idx] & ~0xFFF);
    }

    pt[pt_idx] = pa | 0x3; // Present + RW
}

void paging_bootstrap_identity(void)
{
    kernel_page_directory_phys = pmm_alloc_page();
    if (kernel_page_directory_phys == 0)
    {
        panic("No memory for page directory");
    }

    memset((void*)kernel_page_directory_phys, 0, PAGE_SIZE);

    kernel_page_directory = (uint32_t*)kernel_page_directory_phys;
}

void paging_map_high_half_kernel(void)
{
    uint32_t pd_phys = kernel_page_directory_phys;
    uint32_t *pd = (uint32_t*)pd_phys;   // identity-mapped PD

    uintptr_t kernel_phys_start = (uintptr_t)&_kernel_start;
    uintptr_t kernel_phys_end   = (uintptr_t)&_kernel_end;

    uintptr_t va = KERNEL_VMA;
    uintptr_t pa = kernel_phys_start;

    // Map kernel image high-half
    while (pa < kernel_phys_end) {
        uint32_t pd_idx = (va >> 22) & 0x3FF;
        uint32_t pt_idx = (va >> 12) & 0x3FF;

        if (!(pd[pd_idx] & PAGE_PRESENT)) {
            uint32_t new_pt_pa = pmm_alloc_page();
            memset((void*)new_pt_pa, 0, PAGE_SIZE);
            pd[pd_idx] = new_pt_pa | PAGE_PRESENT | PAGE_RW;
        }

        uint32_t *pt = (uint32_t*)(pd[pd_idx] & 0xFFFFF000); 
        pt[pt_idx] = pa | PAGE_PRESENT | PAGE_RW;

        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }

    // 1) Ensure PD is identity-mapped (you already do this, it's fine)
    map_identity_page((uint32_t*)pd_phys, pd_phys);

    // 2) Map PD into high-half explicitly
    uintptr_t pd_vaddr = pd_phys + KERNEL_VMA;
    uint32_t pd_idx = (pd_vaddr >> 22) & 0x3FF;
    uint32_t pt_idx = (pd_vaddr >> 12) & 0x3FF;

    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        uint32_t new_pt_pa = pmm_alloc_page();
        memset((void*)new_pt_pa, 0, PAGE_SIZE);
        pd[pd_idx] = new_pt_pa | PAGE_PRESENT | PAGE_RW;
    }

    uint32_t *pt = (uint32_t*)(pd[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = pd_phys | PAGE_PRESENT | PAGE_RW;

    // Now this high-half pointer is actually backed by a mapping
    kernel_page_directory = (uint32_t*)pd_vaddr;

    log_info("Paging",
        "High-half kernel mapped successfully: VA=0x%x -> PA=0x%x, kernel_page_directory=%p",
        KERNEL_VMA, kernel_phys_start, kernel_page_directory);
}



// void paging_bootstrap(void)
// {
//     log_info("PAGING", "paging_bootstrap start");

//     // ------------------------
//     // 1) Setup page directory and first PT
//     // ------------------------
//     uint32_t *pd = (uint32_t*)KERNEL_PD_PHYS;
//     uint32_t *pt = (uint32_t*)KERNEL_PT_PHYS;

//     memset(pd, 0, PAGE_SIZE);
//     memset(pt, 0, PAGE_SIZE);

//     // Identity-map first 16 MB for early access
//     for (uint32_t i = 0; i < 4096; i++) {
//         pt[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
//     }

//     pd[0] = KERNEL_PT_PHYS | PAGE_PRESENT | PAGE_RW;

//     // Map high-half kernel using same PT (simplest)
//     pd[768] = KERNEL_PT_PHYS | PAGE_PRESENT | PAGE_RW;

//     // Map VGA memory at high-half (0xC00B8000)
//     pt[0xB8000 >> 12] = 0xB8000 | PAGE_PRESENT | PAGE_RW;

//     // ------------------------
//     // 2) Map entire kernel sections (.text, .data, .bss)
//     // ------------------------
//     uint32_t kernel_phys_start = (uint32_t)&_kernel_start;
//     uint32_t kernel_phys_end   = (uint32_t)&_kernel_end;

//     uint32_t va = KERNEL_VMA + (kernel_phys_start - 0x00100000); // adjust if kernel loaded at 1MB
//     uint32_t pa = kernel_phys_start;

//     while (pa < kernel_phys_end) {
//         uint32_t pd_idx = (va >> 22) & 0x3FF;
//         uint32_t pt_idx = (va >> 12) & 0x3FF;

//         // Allocate new PT if missing
//         if (!(pd[pd_idx] & PAGE_PRESENT)) {
//             uint32_t new_pt_pa = pmm_alloc_page();
//             memset((void*)new_pt_pa, 0, PAGE_SIZE);
//             pd[pd_idx] = new_pt_pa | PAGE_PRESENT | PAGE_RW;
//         }

//         uint32_t *current_pt = (uint32_t*)((pd[pd_idx] & 0xFFFFF000) + KERNEL_VMA);
//         current_pt[pt_idx] = pa | PAGE_PRESENT | PAGE_RW;

//         va += PAGE_SIZE;
//         pa += PAGE_SIZE;
//     }

//     // ------------------------
//     // 3) Map kernel stack
//     // ------------------------
//     uint32_t stack_va = KERNEL_STACK_TOP - KERNEL_STACK_SIZE;
//     while (stack_va < KERNEL_STACK_TOP) {
//         uint32_t pa = pmm_alloc_page();
//         memset((void*)pa, 0, PAGE_SIZE);

//         uint32_t pd_idx = (stack_va >> 22) & 0x3FF;
//         uint32_t pt_idx = (stack_va >> 12) & 0x3FF;

//         if (!(pd[pd_idx] & PAGE_PRESENT)) {
//             uint32_t new_pt_pa = pmm_alloc_page();
//             memset((void*)new_pt_pa, 0, PAGE_SIZE);
//             pd[pd_idx] = new_pt_pa | PAGE_PRESENT | PAGE_RW;
//         }

//         uint32_t *current_pt = (uint32_t*)((pd[pd_idx] & 0xFFFFF000) + KERNEL_VMA);
//         current_pt[pt_idx] = pa | PAGE_PRESENT | PAGE_RW;

//         stack_va += PAGE_SIZE;
//     }

//     // ------------------------
//     // 4) Set globals
//     // ------------------------
//     kernel_page_directory_phys = KERNEL_PD_PHYS;
//     kernel_page_directory = (uint32_t*)(KERNEL_PD_PHYS + KERNEL_VMA);

//     log_info("PAGING", "paging_bootstrap complete, CR3=0x%x", kernel_page_directory_phys);
// }


void enable_paging(void)
{
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    // log_info("PAGING", "enable_paging, cr0 = 0x%x", cr0);

    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
    // log_info("PAGING", "enable_paging, After cr0 |= 0x80000000,  cr0 = 0x%x", cr0);

}
void paging_jump_high(void)
{
    // log_info("PAGING", "paging_jump_high");

    __asm__ volatile (
        "jmp %0"
        :
        : "r"(KERNEL_VMA + (uint32_t)&&high)
    );
high:
    return;
}
uint32_t virt_to_phys(void *virt)
{
    // return (uint32_t)virt - kernel_phys_offset;
    return (uint32_t)virt;
}

void *phys_to_virt(uint32_t phys)
{
    // return (void *)(phys + kernel_phys_offset);
    return (void *)(phys);
}

// uint32_t virt_to_phys(void *virt)
// {
//     return (uint32_t)virt;
// }
// void *phys_to_virt(uint32_t phys)
// {
//     return (void *)(phys);
// }


/* Allocate a zero'd physical page and return its PA,
 * or 0 on failure. This is wrapper around pmm_alloc_page.
 */
// static uint32_t alloc_zeroed_page_phys(void) {
//     uint32_t pa = pmm_alloc_page();
//     if (!pa) return 0;
//     void *kv = phys_to_virt(pa);
//     memset(kv, 0, PAGE_SIZE);
//     return pa;
// }

static inline uint32_t alloc_zeroed_page_phys(void) {
    uint32_t pa = pmm_alloc_page();
    if (!pa) return 0;
    memset(phys_to_virt(pa), 0, PAGE_SIZE); // zero the page via kernel VA
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

// ################################################## Option 1 ####################################

// Map a single physical page temporarily at TEMP_VMA
static void map_temp_page(uintptr_t phys_page, uintptr_t vaddr)
{
    uint32_t pd_idx = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    // Get or create PT for this PDE
    uint32_t *pt;
    // log_info("Paging", "map_temp_page phys_page = 0x%x, vaddr = 0x%x, kernel_page_directory = %p, pd_idx = %u", phys_page, vaddr, kernel_page_directory, pd_idx);

    // uint32_t cr3;
    // __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    // log_info("DBG", "CR3=0x%x, pd_phys=0x%x, kernel_page_directory=%p",
    //         cr3, kernel_page_directory_phys, kernel_page_directory);

    // This should no longer fault
    // log_info("DBG", "PDE[1023]=0x%x", kernel_page_directory[1023]);

    if (!(kernel_page_directory[pd_idx] & 0x1))
    {
        uint32_t pt_pa = pmm_alloc_page();
        // pt = (uint32_t*)(KERNEL_VMA + pt_pa);
        pt = (uint32_t*)(pt_pa);

        // log_info("Paging", "map_temp_page before memset, pt = %p, pt_pa = 0x%x", pt, pt_pa);
        memset(pt, 0, PAGE_SIZE);
        kernel_page_directory[pd_idx] = pt_pa | 0x3; // present+rw
    }
    else
    {
        uint32_t pt_pa = kernel_page_directory[pd_idx] & ~0xFFF;

        log_info("Paging", "map_temp_page else,  kernel_page_directory[pd_idx] = %p, pt_pa = 0x%x",  kernel_page_directory[pd_idx], pt_pa);

        // pt = (uint32_t*)(KERNEL_VMA + pt_pa);
        pt = (uint32_t*)(pt_pa);
    }

    pt[pt_idx] = phys_page | 0x3;  // present+rw
    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

static void map_temp_kernel_pd(uint32_t pd_phys)
{
    // Map the physical page of kernel PD to TEMP_KPD_VMA
    uint32_t pd_idx = (TEMP_KPD_VMA >> 22) & 0x3FF;
    uint32_t pt_idx = (TEMP_KPD_VMA >> 12) & 0x3FF;

    // Allocate page table if missing
    uint32_t *pt;
    if (!(kernel_page_directory[pd_idx] & 0x1)) {
        uint32_t pt_pa = pmm_alloc_page();
        pt = (uint32_t*)(KERNEL_VMA + pt_pa);
        memset(pt, 0, PAGE_SIZE);
        kernel_page_directory[pd_idx] = pt_pa | 0x3;
    } else {
        uint32_t pt_pa = kernel_page_directory[pd_idx] & ~0xFFF;
        pt = (uint32_t*)(KERNEL_VMA + pt_pa);
    }

    // Map the PD physical page at TEMP_KPD_VMA
    pt[pt_idx] = pd_phys | 0x3;
    __asm__ volatile("invlpg (%0)" :: "r"(TEMP_KPD_VMA) : "memory");
}

static void map_temp_page_identity(uint32_t pt_pa, uintptr_t vaddr)
{
    uint32_t pd_idx = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    uint32_t *pt;

    if (!(kernel_page_directory[pd_idx] & 0x1)) {
        uint32_t new_pt_pa = pmm_alloc_page();
        pt = (uint32_t*)(KERNEL_VMA + new_pt_pa);
        memset(pt, 0, PAGE_SIZE);
        kernel_page_directory[pd_idx] = new_pt_pa | 0x3;  // Present+RW
    } else {
        uint32_t pt_pa = kernel_page_directory[pd_idx] & ~0xFFF;
        pt = (uint32_t*)(KERNEL_VMA + pt_pa);
    }

    pt[pt_idx] = pt_pa | 0x3;  // Present+RW
    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

// page_dir_t create_user_pd(void)
// {
//     uint32_t pd_pa    = pmm_alloc_page();        
//     uint32_t *pd_virt = (uint32_t*)TEMP_VMA;

//     map_temp_page(pd_pa, TEMP_VMA);               // map new PD
//     // map_temp_page(KERNEL_PD_PHYS, TEMP_KPD_VMA); // map kernel PD safely

//     memset(pd_virt, 0, PAGE_SIZE);

//     uint32_t *kernel_pd_temp = (uint32_t*)TEMP_KPD_VMA;
//     for (int i = KERNEL_PD_IDX; i < 1024; i++) {
//         pd_virt[i] = kernel_pd_temp[i];
//     }

//     return (page_dir_t){ .pd_phys = pd_pa, .pd_virt = pd_virt };
// }

page_dir_t create_user_pd(void)
{
    uint32_t pd_pa    = pmm_alloc_page();        
    uint32_t *pd_virt = (uint32_t*)TEMP_VMA;

    // Map the *new* PD into TEMP_VMA in the current (kernel) PD
    map_temp_page(pd_pa, TEMP_VMA);

    // Map the *real* kernel PD (the one in CR3) into TEMP_KPD_VMA
    map_temp_page(kernel_page_directory_phys, TEMP_KPD_VMA);

    memset(pd_virt, 0, PAGE_SIZE);

    uint32_t *kernel_pd_temp = (uint32_t*)TEMP_KPD_VMA;
    for (int i = KERNEL_PD_IDX; i < 1024; i++) {
        pd_virt[i] = kernel_pd_temp[i];
    }


    // // Clone low identity mappings (0..32MB) for kernel stack etc.
    // const int IDENTITY_PDE_LIMIT = (32 * 1024 * 1024) / (4 * 1024 * 1024); // 32MB / 4MB = 8

    // for (int i = 0; i < IDENTITY_PDE_LIMIT; i++) {
    //     pd_virt[i] = kernel_pd_temp[i];
    // }

    // // Clone high-half kernel mappings
    // for (int i = KERNEL_PD_IDX; i < 1024; i++) {
    //     pd_virt[i] = kernel_pd_temp[i];
    // }

    // Clone low identity mappings (0..32MB) for kernel stack etc.
    const int IDENTITY_PDE_LIMIT = (32 * 1024 * 1024) / (4 * 1024 * 1024); // 8

    for (int i = 0; i < IDENTITY_PDE_LIMIT; i++) {
        uint32_t pde = kernel_pd_temp[i];

        if (pde & PAGE_PRESENT) {
            // Preserve everything else, just ensure USER bit is set
            pd_virt[i] = pde | PAGE_USER;
        } else {
            pd_virt[i] = 0;
        }
    }

    // Clone high-half kernel mappings (can also force USER if you want user to see them)
    for (int i = KERNEL_PD_IDX; i < 1024; i++) {
        uint32_t pde = kernel_pd_temp[i];
        if (pde & PAGE_PRESENT) {
            pd_virt[i] = pde | PAGE_USER;
        } else {
            pd_virt[i] = 0;
        }
    }


    // log_info("Paging", "create_user_pd end");


    return (page_dir_t){ .pd_phys = pd_pa, .pd_virt = pd_virt };
}


// page_dir_t create_user_pd(void)
// {
//     // 1. Allocate physical page for new user PD
//     uint32_t pd_pa = pmm_alloc_page();
//     uint32_t *pd_virt = (uint32_t*)TEMP_VMA;

//     log_info("EXEC", "create_user_pd: pd_pa = 0x%x, using TEMP_VMA = %p, kernel_page_directory = %p", pd_pa, pd_virt, kernel_page_directory);

//     // 2. Map the user PD temporarily into TEMP_VMA
//     map_temp_page(kernel_page_directory, pd_pa);  // This maps the new PD

//     // // 3. Map the kernel PD itself temporarily so we can read its entries safely
//     // map_temp_page_identity(KERNEL_PD_PHYS);       // maps kernel PD to TEMP_VMA_KPD

//     // uint32_t *kernel_pd_temp = (uint32_t*)TEMP_VMA_KPD;

//     // // 4. Zero the new PD safely
//     // memset(pd_virt, 0, PAGE_SIZE);

//     // // 5. Copy kernel high-half mappings (KERNEL_PD_IDX..1023)
//     // for (int i = KERNEL_PD_IDX; i < 1024; i++) {
//     //     pd_virt[i] = kernel_pd_temp[i];
//     // }

//     // 6. Return physical + virtual pointer for new PD
//     return (page_dir_t){ .pd_phys = pd_pa, .pd_virt = pd_virt };
// }

uint32_t* pt_from_pde(uint32_t pde)
{
    uint32_t pt_pa = pde & 0xFFFFF000;
    return phys_to_virt(pt_pa);
}
int map_page(uint32_t *pd, uintptr_t va, uintptr_t pa, uint32_t flags)
{
    log_info("Paging", "map_page: pd = %p va = 0x%x, pa = 0x%x, flags = %u", pd, va, pa, flags);
    uint32_t pd_idx = (va >> 22) & 0x3FF;
    uint32_t pt_idx = (va >> 12) & 0x3FF;

    uint32_t pde = pd[pd_idx];
    uint32_t *pt;

    if (!(pde & PAGE_PRESENT))
    {

        uintptr_t pt_pa = pmm_alloc_page();
        if (!pt_pa)
        {
            return -1;
        }

        // Zero the physical page via a kernel mapping
        uint8_t *pt_kva = (uint8_t *)phys_to_virt((uint32_t)pt_pa);

        log_info("Paging", "map_page before memset, pt_kva = %p, pt_pa = %p",pt_kva, pt_pa);
        memset(pt_kva, 0, PAGE_SIZE);
        log_info("Paging", "map_page After memset");

        // If phys_to_virt uses TEMP_KVA, unmap after use
        #ifdef TEMP_KVA_USED
        unmap_kernel_temp();
        #endif

        log_info("Paging", "map_page pd_idx = %u",pd_idx);

        // Install PDE with the physical address of the new page table
        pd[pd_idx] = (uint32_t)pt_pa | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        // log_info("Paging", "map_page pd[pd_idx] = %p", pd[pd_idx]);

        // Get a kernel pointer to the page table so we can write PTEs
        pt = (uint32_t *)phys_to_virt((uint32_t)pt_pa);
    }
    else
    {
        uintptr_t pt_pa = pde & 0xFFFFF000;
        pt = (uint32_t *)phys_to_virt((uint32_t)pt_pa);
    }

    // Now write the PTE into the page table via the kernel mapping
    pt[pt_idx] = (pa & 0xFFFFF000) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

    // If phys_to_virt used TEMP_KVA, unmap the page table mapping now
    #ifdef TEMP_KVA_USED
    unmap_kernel_temp();
    #endif

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    return 0;
}


// ##################################################################################################


// ################################################## Option 2 ####################################


// page_dir_t create_user_pd(void)
// {
//     uint32_t pd_pa = pmm_alloc_page();
//     if (!pd_pa) {
//         log_info("Paging", "Out of memory creating user page directory");
//     }

//     // Use identity-mapped virtual for now:
//     uint32_t *pd_virt = (uint32_t*)pd_pa; // ONLY if identity-mapped region includes pd_pa
//     //uint32_t *pd_virt = phys_to_virt(pd_pa);
//     memset(pd_virt, 0, PAGE_SIZE);

//     return (page_dir_t){
//         .pd_phys = pd_pa,
//         .pd_virt = pd_virt
//     };
// }

// uint32_t* pt_from_pde(uint32_t pde)
// {
//     uint32_t pt_pa = pde & 0xFFFFF000;
//     return (uint32_t*)pt_pa; // identity-mapped
// }

// int map_page(uint32_t *pd, uintptr_t va, uintptr_t pa, uint32_t flags)
// {
//     uint32_t pd_idx = (va >> 22) & 0x3FF;
//     uint32_t pt_idx = (va >> 12) & 0x3FF;

//     uint32_t pde = pd[pd_idx];
//     uint32_t *pt;

//     if (!(pde & PAGE_PRESENT)) {
//         uintptr_t pt_pa = pmm_alloc_page();
//         if (!pt_pa)
//         {
//             return -1;
//         }

//         memset((void*)pt_pa, 0, PAGE_SIZE);  // identity

//         pd[pd_idx] = pt_pa | PAGE_PRESENT | PAGE_RW | PAGE_USER;

//         pt = (uint32_t*)pt_pa;               // identity
//     }
//     else
//     {
//         uintptr_t pt_pa = pde & 0xFFFFF000;
//         pt = (uint32_t*)pt_pa;               // identity
//     }

//     pt[pt_idx] = (pa & 0xFFFFF000) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

//     __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");

//     return 0;
// }

// Unmap a single virtual page in the given page directory (kernel PD assumed)
void unmap_page(uint32_t *page_directory, uint32_t va)
{
    uint32_t pd_index = (va >> 22) & 0x3FF;
    uint32_t pt_index = (va >> 12) & 0x3FF;

    uint32_t pde = page_directory[pd_index];
    if (!(pde & 1)) {
        // no page table present
        return;
    }

    uint32_t pt_pa = pde & 0xFFFFF000;
    uint32_t *pt = (uint32_t *)phys_to_virt(pt_pa); // use your phys_to_virt

    pt[pt_index] = 0; // clear PTE

    // Invalidate TLB for this VA
    __asm__ volatile("invlpg (%0)" :: "r"((void*)va) : "memory");
}


// ##################################################################################################


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

void clone_kernel_mappings(uint32_t *user_pd)
{
    // kernel usually mapped in high 1GB: 0xC0000000+
    for (uint32_t i = 768; i < 1024; i++) {
        user_pd[i] = kernel_page_directory[i];
    }
}

extern uint32_t kernel_phys_offset; /* 0xC0000000 typical */

#define SERIAL_PORT_COM1 0x3F8



// /* wait until transmitter holding register empty (bit 5) */
// void early_serial_wait_tx_ready(void) {
//     while (!(i686_inb(SERIAL_PORT_COM1 + 5) & 0x20)) { /* spin */ }
// }

// void early_serial_putc(char c) {
//     early_serial_wait_tx_ready();
//     i686_outb(SERIAL_PORT_COM1 + 0, (uint8_t)c);
// }

// /* print 8-bit nibble as hex char */
// void _put_hex_nibble(uint8_t n) {
//     char c = (n < 10) ? ('0' + n) : ('A' + (n - 10));
//     early_serial_putc(c);
// }

// /* print 32-bit hex, with 0x prefix */
// void early_serial_print_hex32(uint32_t v) {
//     early_serial_write("0x");
//     for (int i = 7; i >= 0; --i) {
//         _put_hex_nibble((v >> (i*4)) & 0xF);
//     }
// }


// void early_serial_write(const char *s) {
//     while (*s) {
//         if (*s == '\n') early_serial_putc('\r'); // optional CR
//         early_serial_putc(*s++);
//     }
// }
/* init COM1 minimal (do once) */
static inline void early_serial_init(void) {
    // i686_outb(SERIAL_PORT_COM1 + 1, 0x00); // disable all interrupts
    // i686_outb(SERIAL_PORT_COM1 + 3, 0x80); // enable DLAB (set baud rate divisor)
    // i686_outb(SERIAL_PORT_COM1 + 0, 0x03); // divisor low byte (38400 baud if 115200 base -> 0x03)
    // i686_outb(SERIAL_PORT_COM1 + 1, 0x00); // divisor high byte
    // i686_outb(SERIAL_PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    // i686_outb(SERIAL_PORT_COM1 + 2, 0xC7); // enable FIFO, clear them, with 14-byte threshold
    // i686_outb(SERIAL_PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void set_kernel_page_directory(void)
{
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));


    uint32_t pd_pa = cr3 & 0xFFFFF000U;


    /* kernel_phys_offset should be the virtual base where physical memory is mapped
       e.g. 0xC0000000 for a high-half kernel. */
    kernel_page_directory = (uint32_t*)(pd_pa + kernel_phys_offset);

    // log_info("VERIFY", "set_kernel_page_directory: cr3=0x%x, pd_pa = 0x%x, kernel_page_directory = %p", cr3, pd_pa, kernel_page_directory);

    /* Do NOT call any log helper here that may touch page tables or allocators.
       If you want to print, use the early-serial routine (see below). */

    //    /* after computing pd_pa above */
    // early_serial_init();
    // early_serial_write("CR3 PA=");
    // early_serial_print_hex32(pd_pa);
    // early_serial_write("\nKPD virt=");
    // early_serial_print_hex32((uint32_t)kernel_page_directory);
    // early_serial_write("\n");

}



// /* early_serial.c — tiny, self-contained serial writer (safe early) */

// #include <stdint.h>

// /* port IO: provide your existing inb/i686_outb if available; else add these */
// static inline void i686_outb(uint16_t port, uint8_t val) {
//     __asm__ volatile ("i686_outb %0, %1" : : "a"(val), "Nd"(port));
// }
// static inline uint8_t inb(uint16_t port) {
//     uint8_t v;
//     __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
//     return v;
// }



// /* tiny integer print (dec) for small numbers, optional */
// static void early_serial_print_u32(uint32_t v) {
//     char buf[16];
//     int pos = 0;
//     if (v == 0) { early_serial_putc('0'); return; }
//     while (v && pos < (int)sizeof(buf)-1) { buf[pos++] = '0' + (v % 10); v /= 10; }
//     while (pos) early_serial_putc(buf[--pos]);
// }

// /* test function you can call directly */
// void early_serial_test_print(void) {
//     early_serial_init();
//     early_serial_write("SERIAL READY\n");
//     early_serial_write("Hello from early serial\n");
//     early_serial_print_hex32(0x1234ABCD);
//     early_serial_write("\n");
// }


// /* early_dump.c — uses early_serial */

// extern void early_serial_init(void);
// extern void early_serial_write(const char *);
// extern void early_serial_print_hex32(uint32_t);
// extern void early_serial_print_u32(uint32_t);



// /* call from the place that earlier did log_info */
// void early_dump_kernel_pd(uint32_t *kernel_pd, uint32_t *user_pd) {
//     early_serial_init();
//     early_serial_write("EARLY DUMP: kernel_page_directory = ");
//     early_serial_print_hex32((uint32_t)kernel_pd);
//     early_serial_write("\n");

//     early_serial_write("EARLY DUMP: user_pd = ");
//     early_serial_print_hex32((uint32_t)user_pd);
//     early_serial_write("\n");

//     /* Dump a few PDEs: 0, 1, 2, 768..772 */
//     for (int i = 0; i < 4; ++i) {
//         early_serial_write("PDE[");
//         early_serial_print_u32(i);
//         early_serial_write("] = ");
//         early_serial_print_hex32(user_pd[i]);
//         early_serial_write("\n");
//     }
//     for (int i = 768; i < 772; ++i) {
//         early_serial_write("PDE[");
//         early_serial_print_u32(i);
//         early_serial_write("] = ");
//         early_serial_print_hex32(user_pd[i]);
//         early_serial_write("\n");
//     }
// }


// void enter_user_mode_from_process(Process *p)
// {
//     if (!p || !p->page_directory) {
//         log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
//         return;
//     }

//     uint32_t pd_phys = virt_to_phys(p->page_directory);
//     if (!pd_phys) {
//         log_error("EXEC", "enter_user_mode: virt_to_phys failed");
//         return;
//     }

//     log_info("EXEC", "enter_user_mode_from_process write_cr3");

//     // early_serial_test_print();

//     // early_dump_kernel_pd(kernel_page_directory, p->page_directory);

//     // /* DEBUG: dump kernel_page_directory and new pd */
//     // log_info("EXEC", "kernel_page_directory = %p", (void*)kernel_page_directory);
//     // log_info("EXEC", "kernel_page_directory = 0x%x", (uint32_t)kernel_page_directory);

//     // dump_pd_info(kernel_page_directory);
//     // dump_pd_info(p->page_directory);

//     /* Load the process page directory (CR3) */
//     write_cr3(pd_phys);

//     log_info("EXEC", "enter_user_mode_from_process After write_cr3");


//     /* Load user data selectors into DS/ES/FS/GS while still in ring0.
//        Loading SS must be done by IRET, so we push SS/ESP in the iret frame below. */
//     __asm__ volatile (
//         "movw %[udsel], %%ax\n\t"
//         "movw %%ax, %%ds\n\t"
//         "movw %%ax, %%es\n\t"
//         "movw %%ax, %%fs\n\t"
//         "movw %%ax, %%gs\n\t"
//         : /* no outputs */
//         : [udsel] "i" (USER_DS)
//         : "ax", "memory"
//     );

//     log_info("EXEC", "enter_user_mode_from_process switch to user mode............");


//     /* Now switch to user mode: build an iret frame and iret.
//        Interrupts should be disabled while manipulating the stack frame */
//     __asm__ volatile (
//         "cli\n\t"                         /* disable interrupts during the switch */
//         "pushl %[udsel]\n\t"             /* SS (user data selector) */
//         "pushl %[esp]\n\t"               /* ESP (user stack top) */
//         "pushf\n\t"                      /* EFLAGS */
//         "pushl %[ucsel]\n\t"             /* CS (user code selector) */
//         "pushl %[eip]\n\t"               /* EIP (entry point) */
//         "iret\n\t"
//         :
//         : [udsel] "r" ((uint32_t)USER_DS),
//           [esp]   "r" ((uint32_t)p->regs.esp),
//           [ucsel] "r" ((uint32_t)USER_CS),
//           [eip]   "r" ((uint32_t)p->regs.eip)
//         : "memory"
//     );

//     /* we should never reach here in the kernel if iret succeeded */
//     log_critical("EXEC", "enter_user_mode: iret returned unexpectedly");
//     for(;;);
// }

void enter_user_mode_from_process(Process *p)
{
    if (!p || !p->page_directory) {
        // log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
        return;

    }

    uint32_t pd_phys = p->cr3;

    if (!pd_phys) {
        log_error("EXEC", "enter_user_mode: virt_to_phys failed");
        return;
    }

    // log_info("EXEC", "enter_user_mode_from_process write_cr3, pd_phys = %p", pd_phys);

    uint32_t k_esp; 
    __asm__ volatile("mov %%esp, %0" : "=r"(k_esp));
    log_info("EXEC", "Before write_cr3: ESP=0x%x, EIP(entry)=0x%x, pd_phys=0x%x", k_esp, p->regs.eip, pd_phys);

    /* Switch to process page directory */
    write_cr3(pd_phys);

    // log_info("EXEC", "enter_user_mode_from_process After write_cr3");

    /*
     * Load user data selectors while still in ring 0.
     * SS MUST NOT be loaded here — it must be loaded by IRET.
     */
    __asm__ volatile (
        "movw %[udsel], %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        :
        : [udsel] "i" (USER_DS)
        : "ax", "memory"
    );

    log_info("EXEC", "enter_user_mode_from_process switching to user mode");

    /*
     * Build a *safe* EFLAGS value for ring 3.
     * We MUST NOT reuse kernel EFLAGS directly.
     */
    uint32_t user_eflags;
    __asm__ volatile (
        "pushf\n\t"
        "pop %0\n\t"
        : "=r"(user_eflags)
    );

    /* Enable interrupts in user mode */
    user_eflags |= (1 << 9);        /* IF = 1 */

    /* Clear IOPL (must be 0 for ring 3) */
    user_eflags &= ~(3 << 12);      /* IOPL = 0 */

    /*
     * Now switch to user mode using IRET.
     * Stack frame layout (top → bottom):
     *   SS
     *   ESP
     *   EFLAGS
     *   CS
     *   EIP
     */
    __asm__ volatile (
        "cli\n\t"                         /* no interrupts during transition */
        "pushl %[udsel]\n\t"             /* SS */
        "pushl %[esp]\n\t"               /* ESP */
        "pushl %[eflags]\n\t"            /* sanitized EFLAGS */
        "pushl %[ucsel]\n\t"             /* CS */
        "pushl %[eip]\n\t"               /* EIP */
        "iret\n\t"
        :
        : [udsel]  "r" ((uint32_t)USER_DS),
          [esp]    "r" ((uint32_t)p->regs.esp),
          [eflags] "r" (user_eflags),
          [ucsel]  "r" ((uint32_t)USER_CS),
          [eip]    "r" ((uint32_t)p->regs.eip)
        : "memory"
    );

    /* We should NEVER reach here */
    log_critical("EXEC", "enter_user_mode: iret returned unexpectedly");
    for (;;);
}
