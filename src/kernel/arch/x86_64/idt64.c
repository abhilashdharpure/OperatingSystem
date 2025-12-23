// idt64.c
#include "idt64.h"
#include "debug.h"
#include "arch/x86_64/serial.h"
#include <util/binary.h>

#define IDT64_ENTRIES 256

__attribute__((section(".text")))
static IDTEntry64 g_IDT64[IDT64_ENTRIES];
static IDTDescriptor64 g_IDT64Descriptor = { sizeof(g_IDT64) - 1, g_IDT64 };

static inline void lidt64(IDTDescriptor64* desc)
{
    __asm__ volatile("lidt %0" : : "m"(*desc));
}

extern void x64_isr_common(void);   // 64-bit ASM stub, see below

void x64_IDT_SetGate(int interrupt, void* base, uint16_t segmentSelector, uint8_t flags, uint8_t ist)
{
    uint64_t addr = (uint64_t)base;

    g_IDT64[interrupt].OffsetLow  = (uint16_t)(addr & 0xFFFF);
    g_IDT64[interrupt].SegmentSelector = segmentSelector;
    g_IDT64[interrupt].Ist       = ist & 0x7;
    g_IDT64[interrupt].Flags     = flags; // e.g. IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT
    g_IDT64[interrupt].OffsetMid = (uint16_t)((addr >> 16) & 0xFFFF);
    g_IDT64[interrupt].OffsetHigh= (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    g_IDT64[interrupt].Reserved  = 0;
}

// void dump_syscall_idt_entry(void)
// {
//     log_info("SYSCALL", "IDT[0x80].Flags=0x%02x BaseLo=0x%04x BaseHi=0x%04x",
//              g_IDT[0x80].Flags,
//              g_IDT[0x80].BaseLow,
//              g_IDT[0x80].BaseHigh);

//     uint32_t idt_addr = g_IDT[0x80].BaseLow | (g_IDT[0x80].BaseHigh << 16);
//     log_info("SYSCALL", "IDT[0x80] addr = 0x%x flags=0x%x", idt_addr, g_IDT[0x80].Flags);

//     uint32_t isr_addr = (uint32_t)i686_ISR128;
//     log_info("SYSCALL", "symbol i686_ISR128 = 0x%x", isr_addr);
// }

void x64_IDT_Initialize(void)
{
    // Clear all entries
    for (int i = 0; i < IDT64_ENTRIES; ++i)
    {
        g_IDT64[i].OffsetLow   = 0;
        g_IDT64[i].SegmentSelector = 0;
        g_IDT64[i].Ist         = 0;
        g_IDT64[i].Flags       = 0;
        g_IDT64[i].OffsetMid   = 0;
        g_IDT64[i].OffsetHigh  = 0;
        g_IDT64[i].Reserved    = 0;
    }

    // For now, route CPU exceptions 0..31 to a single common stub
    uint8_t flags = IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT; // 0x80 | 0x00 | 0xE = 0x8E
    for (int vec = 0; vec < 32; ++vec)
    {
        x64_IDT_SetGate(vec, x64_isr_common, 0x08, flags, 0);
    }

    lidt64(&g_IDT64Descriptor);

    log_info("IDT64", "Loaded 64-bit IDT at %p (limit=%u)",
             (void*)g_IDT64Descriptor.Ptr, g_IDT64Descriptor.Limit);
}

void x64_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT64[interrupt].Flags, IDT_FLAG_PRESENT);
}

void x64_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT64[interrupt].Flags, IDT_FLAG_PRESENT);

}
