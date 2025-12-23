#include "tss.h"
#include "arch/x86_64/gdt.h"   /* we will write GDT entry into g_GDT[] */
#include <string.h>
#include <debug.h>

#define IO_BITMAP_SIZE 8192  // 65536 ports / 8

extern GDTEntry g_GDT[];
extern GDTR g_GDT_Ptr;

/* 32-bit TSS structure (minimal fields we need) */
typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_entry_t;

// typedef struct __attribute__((packed)) {
//     uint32_t reserved0;
//     uint64_t rsp0;
//     uint64_t rsp1;
//     uint64_t rsp2;
//     uint64_t reserved1;
//     uint64_t ist1;
//     uint64_t ist2;
//     uint64_t ist3;
//     uint64_t ist4;
//     uint64_t ist5;
//     uint64_t ist6;
//     uint64_t ist7;
//     uint64_t reserved2;
//     uint16_t reserved3;
//     uint16_t iomap_base;
// } tss64_t;


/* allocate TSS in .bss/static */
static tss_entry_t the_tss;

/* TSS selector index in GDT: pick next free index (we used indices 0..4 already) */
#define TSS_GDT_INDEX 5
#define TSS_SELECTOR   ((TSS_GDT_INDEX << 3) | 0) /* RPL = 0 */

uint16_t i686_TSS_Selector(void) { return (uint16_t)TSS_SELECTOR; }

// /* helper to create a GDT descriptor (for TSS) */
// static void gdt_set_descriptor(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
// {
//     /* each GDT entry is 8 bytes in g_GDT[] */
//     uint8_t *p = (uint8_t*)&g_GDT[index];

//     /* low dword */
//     *((uint16_t*)&p[0]) = (uint16_t)(limit & 0xFFFF);       /* limit low */
//     *((uint16_t*)&p[2]) = (uint16_t)(base & 0xFFFF);        /* base low */

//     /* high dword */
//     p[4] = (uint8_t)((base >> 16) & 0xFF);                  /* base middle */
//     p[5] = access;                                         /* access byte */
//     p[6] = (uint8_t)(((limit >> 16) & 0x0F) | (flags & 0xF0)); /* flags + limit hi */
//     p[7] = (uint8_t)((base >> 24) & 0xFF);                 /* base high */
// }

void gdt_set_tss64_descriptor(int index, uint64_t base, uint32_t limit)
{
    // Fill first GDT entry
    g_GDT[index].limit_low   = limit & 0xFFFF;
    g_GDT[index].base_low    = base & 0xFFFF;
    g_GDT[index].base_mid = (base >> 16) & 0xFF;
    g_GDT[index].access      = 0x89;  // present, type 9 (64-bit TSS)
    g_GDT[index].flags       = (limit >> 16) & 0x0F;
    g_GDT[index].base_high   = (base >> 24) & 0xFF;

    // Upper 32 bits of base go into next GDT entry
    g_GDT[index + 1].base_low  = (base >> 32) & 0xFFFF;
    g_GDT[index + 1].base_mid = (base >> 48) & 0xFF;
    g_GDT[index + 1].base_high = 0;
    g_GDT[index + 1].flags     = 0;
    g_GDT[index + 1].access    = 0;

}


struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


static inline void reload_gdt(void)
{
    __asm__ volatile ("lgdt %0" :: "m"(g_GDT_Ptr));
}

void x64_TSS_Install(uintptr_t stack_top) {
    memset(&the_tss, 0, sizeof(the_tss));
    the_tss.rsp0 = stack_top;

    gdt_set_tss64_descriptor(TSS_GDT_INDEX, (uint64_t)&the_tss, sizeof(the_tss) - 1);

    gdt_flush((uint64_t)&g_GDT); // reload GDT
    __asm__ volatile("ltr %%ax" :: "a"(TSS_SELECTOR));
}


// void i686_TSS_Install(uintptr_t stack_top)
// {
//     memset(&the_tss, 0, sizeof(the_tss));

//     // the_tss.esp0 = stack_top;
//     // the_tss.esp0 = (uintptr_t)&_kernel_stack_top - 4;
//     the_tss.esp0 = (uintptr_t)&stack_top + 16384; // top of 16 KiB stack

//     the_tss.ss0  = KERNEL_DATA_SELECTOR;

//     the_tss.iomap_base = offsetof(tss_entry_t, io_bitmap);

//     memset(the_tss.io_bitmap, 0xFF, IO_BITMAP_SIZE);

//     /* Allow port 0xE9 */
//     the_tss.io_bitmap[0xE9 / 8] &= ~(1 << (0xE9 % 8));

//     gdt_set_descriptor(
//         TSS_GDT_INDEX,
//         (uint32_t)&the_tss,
//         sizeof(the_tss) - 1,
//         0x89,   // Present | 32-bit TSS
//         0x00
//     );

//     reload_gdt();

//     /* Now safe */
//     __asm__ volatile ("ltr %%ax" :: "a"(TSS_SELECTOR));
// }
