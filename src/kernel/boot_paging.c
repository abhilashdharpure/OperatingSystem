// boot_paging.c – used only before switch_to_high_pml4

#include <boot_paging.h>
#include <stdint.h>


extern char _kernel_start;
extern char _kernel_end;

/* ---------- tiny boot allocator (identity-mapped) ---------- */


extern uint8_t _boot_stub_end;


__attribute__((section(".boot64_stub_data")))
static uint64_t boot_next_page_pa = 0;

__attribute__((section(".boot64_stub")))
void boot_pmm_init(void)
{
    uint64_t start = (uint64_t)&_boot_stub_end;
    start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (start < BOOT_PMM_START_PA)
        start = BOOT_PMM_START_PA;

    // --- NEW: skip over kernel image in physical memory ---
    uint64_t kernel_va_start = (uint64_t)&_kernel_start;
    uint64_t kernel_va_end   = (uint64_t)&_kernel_end;
    uint64_t kernel_size     = kernel_va_end - kernel_va_start;
    uint64_t kernel_phys_end = KERNEL_LMA_BASE + kernel_size;

    if (start < kernel_phys_end)
        start = (kernel_phys_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    // ------------------------------------------------------

    boot_next_page_pa = start;
}



__attribute__((section(".boot64_stub")))
static inline void write_cr3_boot(uint64_t phys)
{
    __asm__ volatile("mov %0, %%cr3" :: "r"(phys) : "memory");
}

__attribute__((section(".boot64_stub")))
void switch_to_high_pml4(uint64_t *pml4)
{
    test_serial_putc_asm('A');

    uint64_t phys = (uint64_t)pml4;   // identity: VA == PA

    write_cr3_boot(phys);

    test_serial_putc_asm('B');
}


__attribute__((section(".boot64_stub")))
uint64_t boot_alloc_page(void)
{
    uint64_t pa = boot_next_page_pa;
    boot_next_page_pa += PAGE_SIZE;
    if (boot_next_page_pa > BOOT_PMM_LIMIT_PA)
        return 0;
    return pa;
}

/* ---------- identity PDPT for low 1 GiB ---------- */

__attribute__((section(".boot64_stub_data")))
uint64_t boot_pdpt_identity[512] __attribute__((aligned(4096)));

/* ---------- helpers to walk/allocate paging levels ---------- */

__attribute__((section(".boot64_stub")))
static uint64_t *boot_get_or_alloc_pdp(uint64_t *pml4, uint64_t va, uint64_t flags)
{
    uint64_t idx = PML4_INDEX(va);
    uint64_t e   = pml4[idx];

    if (!(e & PAGE_PRESENT)) {
        uint64_t pa = boot_alloc_page();
        if (!pa) return 0;

        uint64_t *pdp = (uint64_t *)pa;   // identity: VA == PA
        for (int i = 0; i < 512; i++) pdp[i] = 0;

        pml4[idx] = pa | flags | PAGE_PRESENT;
        return pdp;
    } else {
        uint64_t pa = e & PTE_ADDR_MASK;
        return (uint64_t *)pa;
    }
}

__attribute__((section(".boot64_stub")))
static uint64_t *boot_get_or_alloc_pd(uint64_t *pdp, uint64_t va, uint64_t flags)
{
    uint64_t idx = PDP_INDEX(va);
    uint64_t e   = pdp[idx];

    if (e & PAGE_PS) {
        return 0; // don’t split huge pages here
    }

    if (!(e & PAGE_PRESENT)) {
        uint64_t pa = boot_alloc_page();
        if (!pa) return 0;

        uint64_t *pd = (uint64_t *)pa;
        for (int i = 0; i < 512; i++) pd[i] = 0;

        pdp[idx] = pa | flags | PAGE_PRESENT;
        return pd;
    } else {
        uint64_t pa = e & PTE_ADDR_MASK;
        return (uint64_t *)pa;
    }
}

__attribute__((section(".boot64_stub")))
static uint64_t *boot_get_or_alloc_pt(uint64_t *pd, uint64_t va, uint64_t flags)
{
    uint64_t idx = PD_INDEX(va);
    uint64_t e   = pd[idx];

    if (!(e & PAGE_PRESENT)) {
        uint64_t pa = boot_alloc_page();
        if (!pa) return 0;

        uint64_t *pt = (uint64_t *)pa;
        for (int i = 0; i < 512; i++) pt[i] = 0;

        pd[idx] = pa | flags | PAGE_PRESENT;
        return pt;
    } else {
        uint64_t pa = e & PTE_ADDR_MASK;
        return (uint64_t *)pa;
    }
}

/* ---------- public boot mapping API ---------- */

__attribute__((section(".boot64_stub")))
int boot_map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags)
{
    uint64_t *pdp = boot_get_or_alloc_pdp(pml4, va, flags);
    if (!pdp) return -1;

    uint64_t *pd = boot_get_or_alloc_pd(pdp, va, flags);
    if (!pd) return -1;

    uint64_t *pt = boot_get_or_alloc_pt(pd, va, flags);
    if (!pt) return -1;

    uint64_t idx = PT_INDEX(va);
    pt[idx] = (pa & PTE_ADDR_MASK) | (flags & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

    __asm__ volatile("invlpg (%0)" :: "r"(va) : "memory");
    return 0;
}



/* tiny boot allocator, identity-mapped ... (unchanged) */
/* boot_pmm_init, boot_alloc_page, boot_get_or_alloc_* etc. unchanged */

__attribute__((section(".boot64_stub")))
void boot_setup_pml4(uint64_t *pml4)
{
    for (int i = 0; i < 512; i++) pml4[i] = 0;

    // low 1 GiB identity via 1 GiB huge page
    boot_pdpt_identity[0] = 0x0000000000000083ULL; // P | RW | PS
    for (int i = 1; i < 512; i++)
        boot_pdpt_identity[i] = 0;

    uint64_t pdpt_pa = (uint64_t)boot_pdpt_identity; // identity
    pml4[0] = pdpt_pa | PAGE_PRESENT | PAGE_RW;
}

__attribute__((section(".boot64_stub")))
void setup_high_mappings(uint64_t *pml4, BootParams *bp)
{
    //boot_setup_pml4(pml4);  // low 1 GiB identity

    // 1) Map kernel higher-half
    uint64_t kernel_va_start = (uint64_t)&_kernel_start;
    uint64_t kernel_va_end   = (uint64_t)&_kernel_end;
    uint64_t kernel_pa_start = KERNEL_LMA_BASE;
    uint64_t size            = kernel_va_end - kernel_va_start;

    for (uint64_t off = 0; off < size; off += PAGE_SIZE) {
        uint64_t va = kernel_va_start + off;
        uint64_t pa = kernel_pa_start + off;
        boot_map_page(pml4, va, pa, PAGE_PRESENT | PAGE_RW);
    }

    // 2) Compute max physical address from BootParams
    uint64_t max_phys = 0;
    for (uint32_t i = 0; i < bp->Memory.RegionCount; i++) {
        MemoryRegion *r = &bp->Memory.Regions[i];
        if (r->Type != 1)  // usable RAM only
            continue;
        uint64_t end = r->Begin + r->Length;
        if (end > max_phys)
            max_phys = end;
    }

    // 3) Boot-time direct map: only up to BOOT_PMM_LIMIT_PA
    uint64_t limit = max_phys;
    if (limit > BOOT_PMM_LIMIT_PA)
        limit = BOOT_PMM_LIMIT_PA;

    for (uint64_t pa = 0; pa < limit; pa += PAGE_SIZE) {
        uint64_t va = DIRECT_MAP_BASE + pa;
        boot_map_page(pml4, va, pa, PAGE_PRESENT | PAGE_RW);
    }
}

uint64_t boot_get_boot_alloc_end(void)
{
    return boot_next_page_pa;
}