#include "gdt.h"
#include <stdint.h>

#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

GDTEntry g_GDT[] = {
    {0,0,0,0,0,0},
    /* kernel code */
    GDT_ENTRY(0, 0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),
    /* kernel data */
    GDT_ENTRY(0, 0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),
    /* user code */
    GDT_ENTRY(0, 0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),
    /* user data */
    GDT_ENTRY(0, 0xFFFFF,
        GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
        GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),
    /* TSS (filled later) */
    GDT_ENTRY(0, 0, 0, 0),
};

GDTR g_GDT_Ptr = {
    .limit = sizeof(g_GDT) - 1,
    .base  = (uint32_t)&g_GDT
};

// GDTDescriptor g_GDTDescriptor = { sizeof(g_GDT) - 1, g_GDT};

// void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);

// void i686_GDT_Initialize()
// {
//     i686_GDT_Load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
// }

/* exported size for loader */
uint32_t g_GDT_size = sizeof(g_GDT);


void i686_GDT_Initialize(void)
{
    __asm__ volatile ("lgdt %0" :: "m"(g_GDT_Ptr) : "memory");

    __asm__ volatile (
        "mov %[ds], %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        :
        : [ds] "i"(KERNEL_DATA_SELECTOR)
        : "ax", "memory"
    );

    __asm__ volatile (
        "ljmp %[cs], $1f\n"
        "1:\n"
        :
        : [cs] "i"(KERNEL_CODE_SELECTOR)
        : "memory"
    );
}