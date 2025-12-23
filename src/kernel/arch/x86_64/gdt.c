#include "gdt.h"

extern void gdt_flush(uint64_t gdtr);
extern void tss_flush(uint16_t selector);

static TSSDescriptor tss_desc;
GDTEntry g_GDT[6];   // add 6th entry for TSS
GDTR g_GDT_Ptr;

static inline GDTEntry gdt_make_entry(uint8_t access, uint8_t flags)
{
    return (GDTEntry){
        .limit_low = 0,
        .base_low  = 0,
        .base_mid  = 0,
        .access    = access,
        .flags     = flags,
        .base_high = 0
    };
}

void gdt_init(void)
{
    g_GDT[0] = gdt_make_entry(0, 0);

    g_GDT[1] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
        GDT_FLAG_LONG_MODE
    );

    g_GDT[2] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
        0
    );

    g_GDT[3] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
        GDT_FLAG_LONG_MODE
    );

    g_GDT[4] = gdt_make_entry(
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
        0
    );

    // TSS entry placeholder, will be set later by tss.c
    g_GDT[5] = gdt_make_entry(0, 0);

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
