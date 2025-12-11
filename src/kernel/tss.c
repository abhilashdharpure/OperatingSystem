#include "tss.h"
#include "arch/i686/gdt.h"   /* we will write GDT entry into g_GDT[] */
#include <string.h>

/* 32-bit TSS structure (minimal fields we need) */
typedef struct __attribute__((packed)) {
    uint32_t prev_task;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} tss_entry_t;

/* allocate TSS in .bss/static */
static tss_entry_t the_tss;

/* TSS selector index in GDT: pick next free index (we used indices 0..4 already) */
#define TSS_GDT_INDEX 5
#define TSS_SELECTOR   ((TSS_GDT_INDEX << 3) | 0) /* RPL = 0 */

uint16_t i686_TSS_Selector(void) { return (uint16_t)TSS_SELECTOR; }

/* helper to create a GDT descriptor (for TSS) */
static void gdt_set_descriptor(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    /* each GDT entry is 8 bytes in g_GDT[] */
    uint8_t *p = (uint8_t*)&g_GDT[index];

    /* low dword */
    *((uint16_t*)&p[0]) = (uint16_t)(limit & 0xFFFF);       /* limit low */
    *((uint16_t*)&p[2]) = (uint16_t)(base & 0xFFFF);        /* base low */

    /* high dword */
    p[4] = (uint8_t)((base >> 16) & 0xFF);                  /* base middle */
    p[5] = access;                                         /* access byte */
    p[6] = (uint8_t)(((limit >> 16) & 0x0F) | (flags & 0xF0)); /* flags + limit hi */
    p[7] = (uint8_t)((base >> 24) & 0xFF);                 /* base high */
}

// void i686_TSS_Install(uintptr_t stack_top)
// {
//     memset(&the_tss, 0, sizeof(the_tss));
//     the_tss.ss0 = I686_GDT_KERNEL_DATA_SEL;
//     the_tss.esp0 = stack_top;

//     g_GDT[5].LimitLow     = GDT_LIMIT_LOW(sizeof(the_tss)-1);
//     g_GDT[5].BaseLow      = GDT_BASE_LOW((uint32_t)&the_tss);
//     g_GDT[5].BaseMiddle   = GDT_BASE_MIDDLE((uint32_t)&the_tss);
//     g_GDT[5].Access       = GDT_ACCESS_PRESENT | GDT_ACCESS_DESCRIPTOR_TSS | GDT_ACCESS_RING0; // Ring0 TSS
//     g_GDT[5].FlagsLimitHi = GDT_FLAGS_LIMIT_HI(sizeof(the_tss)-1, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_1B);
//     g_GDT[5].BaseHigh     = GDT_BASE_HIGH((uint32_t)&the_tss);

//     // Load TR (TSS)
//     // __asm__ volatile("ltr %0" : : "a"(I686_GDT_TSS_SEL));
//     __asm__ volatile("ltr %%ax" : : "a"(I686_GDT_TSS_SEL));
// }
void i686_TSS_Install(uintptr_t stack_top)
{
    memset(&the_tss, 0, sizeof(the_tss));
    the_tss.esp0 = stack_top;
    the_tss.ss0  = KERNEL_DATA_SELECTOR;

    g_GDT[5].BaseLow      = GDT_BASE_LOW((uint32_t)&the_tss);
    g_GDT[5].BaseMiddle   = GDT_BASE_MIDDLE((uint32_t)&the_tss);
    g_GDT[5].BaseHigh     = GDT_BASE_HIGH((uint32_t)&the_tss);
    g_GDT[5].Access       = GDT_ACCESS_PRESENT | 0x9 | GDT_ACCESS_RING0; // 32-bit available TSS
    g_GDT[5].FlagsLimitHi = GDT_FLAGS_LIMIT_HI(sizeof(the_tss)-1, GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_1B);

    __asm__ volatile("ltr %%ax" :: "a"(I686_GDT_TSS_SEL));
}
