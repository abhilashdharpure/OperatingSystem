#include "gdt.h"
#include <stdint.h>

// typedef struct
// {
//     uint16_t LimitLow;                  // limit (bits 0-15)
//     uint16_t BaseLow;                   // base (bits 0-15)
//     uint8_t BaseMiddle;                 // base (bits 16-23)
//     uint8_t Access;                     // access
//     uint8_t FlagsLimitHi;               // limit (bits 16-19) | flags
//     uint8_t BaseHigh;                   // base (bits 24-31)
// } __attribute__((packed)) GDTEntry;




// typedef struct
// {
//     uint16_t LimitLow;                  // limit (bits 0-15)
//     uint16_t BaseLow;                   // base (bits 0-15)
//     uint8_t BaseMiddle;                 // base (bits 16-23)
//     uint8_t Access;                     // access
//     uint8_t FlagsLimitHi;               // limit (bits 16-19) | flags
//     uint8_t BaseHigh;                   // base (bits 24-31)
// } __attribute__((packed)) GDTEntry;






#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

GDTEntry g_GDT[] = {
    // NULL descriptor
    GDT_ENTRY(0, 0, 0, 0),

    // Kernel 32-bit code segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // Kernel 32-bit data segment
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // User 32-bit code segment (index 3) -> selector with RPL=3 will be 0x1B
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // User 32-bit data segment (index 4) -> selector with RPL=3 will be 0x23
    GDT_ENTRY(0,
              0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // Reserved for TSS, it get update in tss.c
    GDT_ENTRY(0, 0, 0, 0),

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
    GDTR gdtr;
    gdtr.limit = (uint16_t)(g_GDT_size - 1);
    gdtr.base  = (uint32_t)&g_GDT[0];

    /* Load GDTR */
    __asm__ volatile ("lgdt (%0)" :: "r"(&gdtr) : "memory");

    /* Reload data segments and do a far return/jump to reload CS.
       We'll use retf trick: push offset then push selector then lret. */

    /* kernel data selector to load DS/ES/FS/GS */
    __asm__ volatile(
        "mov %[kds], %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        :
        : [kds] "i" (KERNEL_DATA_SELECTOR)
        : "ax", "memory"
    );

    /* far jump/return to reload CS (push offset then selector and lret) */
    __asm__ volatile(
        "pushl %[kcs]\n\t"   /* push kernel CS selector */
        "pushl $1f\n\t"      /* push return eip (label 1) */
        "lret\n\t"           /* pop eip, cs -> effectively a far jump */
        "1:\n\t"
        :
        : [kcs] "i" (KERNEL_CODE_SELECTOR)
        : "memory"
    );
}
