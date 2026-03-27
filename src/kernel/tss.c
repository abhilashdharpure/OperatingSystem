#include <stdint.h>
#include <string.h>
#include "arch/x86_64/gdt.h"
#include "debug.h"

extern GDTEntry g_GDT[];
extern GDTR g_GDT_Ptr;

#define TSS_GDT_INDEX 5
#define TSS_SELECTOR   ((TSS_GDT_INDEX << 3) | 0)  // RPL=0

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

static tss_entry_t the_tss;

/* Put a 64-bit TSS descriptor into g_GDT at index `index`.
   This writes 16 bytes (index and index+1). */
void gdt_set_tss64_descriptor(int index, uint64_t base, uint32_t limit)
{
    uint64_t desc_low = 0;
    uint64_t desc_high = 0;

    // descriptor low 64 bits
    desc_low  = (uint64_t)(limit & 0xFFFFULL);                 // limit[0:15]
    desc_low |= (uint64_t)(base & 0xFFFFULL) << 16;            // base[0:15]
    desc_low |= (uint64_t)((base >> 16) & 0xFFULL) << 32;      // base[16:23]
    desc_low |= (uint64_t)0x89ULL << 40;                       // access: P=1, type=9 (available 64-bit TSS) => 1000 1001 = 0x89
    desc_low |= (uint64_t)((limit >> 16) & 0xFULL) << 48;      // limit[16:19]
    desc_low |= (uint64_t)0ULL << 52;                         // flags (G, D/B, L, AVL) -> for TSS L=0, D/B=0, G maybe 0
    desc_low |= (uint64_t)((base >> 24) & 0xFFULL) << 56;     // base[24:31]

    // descriptor high 64 bits: base[32:63] in low dword, rest zero
    desc_high = (uint64_t)((base >> 32) & 0xFFFFFFFFULL);

    // Write the two u64s into the GDT array. We assume g_GDT is properly sized and aligned.
    uint64_t *gdt_u64 = (uint64_t *)&g_GDT[index];
    gdt_u64[0] = desc_low;
    gdt_u64[1] = desc_high;

    log_info("TSS", "TSS descriptor written: index=%d base=%p limit=%u", index, (void*)base, limit);
}

// exported for syscall64.asm
uint64_t g_syscall_rsp0;

void x64_TSS_Install(uintptr_t stack_top) {
    log_info("TSS", "Start x64_TSS_Install");

    memset(&the_tss, 0, sizeof(the_tss));
    the_tss.rsp0 = stack_top;
    the_tss.iomap_base = sizeof(the_tss);

    g_syscall_rsp0 = stack_top;   // <-- this is what syscall64.asm will use

    log_info("TSS", "g_syscall_rsp0 = 0x%llx", (unsigned long long)g_syscall_rsp0);
    // run at boot or module init
    if ((g_syscall_rsp0 & 0xF) != 0)
    {
        panic("g_syscall_rsp0 not 16-byte aligned");
    }

    gdt_set_tss64_descriptor(TSS_GDT_INDEX, (uint64_t)&the_tss, sizeof(the_tss) - 1);

    log_info("TSS", "Reloading GDT");
    gdt_flush((uint64_t)&g_GDT_Ptr);

    log_info("TSS", "Loading TSS selector 0x%x", TSS_SELECTOR);
    __asm__ volatile ("ltr %0" :: "r"((uint16_t)TSS_SELECTOR));
    log_info("TSS", "TSS installed at %p, rsp0=0x%llx", &the_tss, (unsigned long long)the_tss.rsp0);
}
