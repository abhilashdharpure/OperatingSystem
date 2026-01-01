#include "gdt.h"

extern void gdt_flush(uint64_t gdtr);
extern void tss_flush(uint16_t selector);

static TSSDescriptor tss_desc;
GDTEntry g_GDT[7];   // add 6th entry for TSS
GDTR g_GDT_Ptr;

static inline GDTEntry gdt_make_entry(uint8_t access, uint8_t flags)
{
    // flags: bits 0-3 -> limit high (usually 0), bits 4-7 -> actual flags (L, D, G)
    uint8_t gran = (flags & 0xF0);  // limit_high = 0, flags in high nibble

    return (GDTEntry){
        .limit_low = 0,
        .base_low  = 0,
        .base_mid  = 0,
        .access    = access,
        .gran      = gran,
        .base_high = 0
    };
}

void gdt_init(void)
{
    uint64_t* g = (uint64_t*)g_GDT;

    // 0: null
    g[0] = 0x0000000000000000ull;

    // 1: kernel code (base=0, limit=0, 64-bit, DPL=0, RW)
    g[1] = 0x00209A0000000000ull;

    // 2: kernel data (base=0, limit=0, DPL=0, RW)
    g[2] = 0x0000920000000000ull;

    // 3: user code (DPL=3, 64-bit, RW)
    g[3] = 0x0020FA0000000000ull;

    // 4: user data (DPL=3, RW)
    g[4] = 0x0000F20000000000ull;

    // 5 + 6: zero for now (TSS placeholder)
    g[5] = 0x0000000000000000ull;
    g[6] = 0x0000000000000000ull;

    g_GDT_Ptr.limit = sizeof(g_GDT) - 1;
    g_GDT_Ptr.base  = (uint64_t)&g_GDT;

    gdt_flush((uint64_t)&g_GDT_Ptr);

    __asm__ volatile (
        "mov %0, %%ds\n"
        "mov %0, %%es\n"
        "mov %0, %%ss\n"
        :
        : "r"(KERNEL_DATA_SELECTOR)
        : "memory"
    );
}
