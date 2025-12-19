#include "paging.h"
#include <string.h>
#include <stdio.h>
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

extern uint32_t pmm_alloc_page(void);      /* must be provided by your PMM */
extern void phys_free_page(uint32_t pa);
extern void *phys_to_virt(uint32_t pa);     /* kernel virtual address for physical page */
extern uint32_t virt_to_phys(void *v);      /* optional */

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
    while (pa < kernel_phys_end)
    {
        uint32_t pd_idx = (va >> 22) & 0x3FF;
        uint32_t pt_idx = (va >> 12) & 0x3FF;

        if (!(pd[pd_idx] & PAGE_PRESENT))
        {
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

    if (!(pd[pd_idx] & PAGE_PRESENT))
    {
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

void enable_paging(void)
{
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

uint32_t virt_to_phys(void *virt)
{
    return (uint32_t)virt;
}

void *phys_to_virt(uint32_t phys)
{
    return (void *)(phys);
}

static inline uint32_t alloc_zeroed_page_phys(void)
{
    uint32_t pa = pmm_alloc_page();
    if (!pa) return 0;
    memset(phys_to_virt(pa), 0, PAGE_SIZE); // zero the page via kernel VA
    return pa;
}

// Map a single physical page temporarily at TEMP_VMA
static void map_temp_page(uintptr_t phys_page, uintptr_t vaddr)
{
    uint32_t pd_idx = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    // Get or create PT for this PDE
    uint32_t *pt;
    if (!(kernel_page_directory[pd_idx] & 0x1))
    {
        uint32_t pt_pa = pmm_alloc_page();
        pt = (uint32_t*)(pt_pa);
        memset(pt, 0, PAGE_SIZE);
        kernel_page_directory[pd_idx] = pt_pa | 0x3; // present+rw
    }
    else
    {
        uint32_t pt_pa = kernel_page_directory[pd_idx] & ~0xFFF;
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
    for (int i = KERNEL_PD_IDX; i < 1024; i++)
    {
        pd_virt[i] = kernel_pd_temp[i];
    }

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
    for (int i = KERNEL_PD_IDX; i < 1024; i++)
    {
        uint32_t pde = kernel_pd_temp[i];
        if (pde & PAGE_PRESENT)
        {
            pd_virt[i] = pde | PAGE_USER;
        } else {
            pd_virt[i] = 0;
        }
    }

    return (page_dir_t){ .pd_phys = pd_pa, .pd_virt = pd_virt };
}

uint32_t* pt_from_pde(uint32_t pde)
{
    uint32_t pt_pa = pde & 0xFFFFF000;
    return phys_to_virt(pt_pa);
}

int map_page(uint32_t *pd, uintptr_t va, uintptr_t pa, uint32_t flags)
{
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
        memset(pt_kva, 0, PAGE_SIZE);

        // If phys_to_virt uses TEMP_KVA, unmap after use
        #ifdef TEMP_KVA_USED
        unmap_kernel_temp();
        #endif

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

void clone_kernel_mappings(uint32_t *user_pd)
{
    // kernel usually mapped in high 1GB: 0xC0000000+
    for (uint32_t i = 768; i < 1024; i++)
    {
        user_pd[i] = kernel_page_directory[i];
    }
}

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
