#include "idt.h"
#include <stdint.h>
#include <util/binary.h>
#include "debug.h"

typedef struct
{
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t Limit;
    IDTEntry* Ptr;
} __attribute__((packed)) IDTDescriptor;

extern void i686_ISR128(void);   // declare assembly label

IDTEntry g_IDT[256];

IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);

// void dump_syscall_idt_entry(void)
// {
//     // extern IDTEntry g_IDT[]; // visible in idt.c
//     log_info("SYSCALL", "IDT[0x80].Flags=0x%02x BaseLo=0x%04x BaseHi=0x%04x",
//              g_IDT[0x80].Flags,
//              g_IDT[0x80].BaseLow,
//              g_IDT[0x80].BaseHigh);

//     log_info("SYSCALL", "Test: IDT[0x80].Flags=%u BaseLo=%u BaseHi=%u",
//              g_IDT[0x80].Flags,
//              g_IDT[0x80].BaseLow,
//              g_IDT[0x80].BaseHigh);


//     uint32_t isr_addr = (uint32_t)i686_ISR128;
//     uint32_t idt_addr  = g_IDT[128].BaseLow | (g_IDT[128].BaseHigh << 16);

//     log_info("SYSCALL", "symbol i686_ISR128 = 0x%x", isr_addr);
//     log_info("SYSCALL", "IDT[128] addr = 0x%x flags=0x%x", idt_addr, g_IDT[128].Flags);
// }

void dump_syscall_idt_entry(void)
{
    log_info("SYSCALL", "IDT[0x80].Flags=0x%02x BaseLo=0x%04x BaseHi=0x%04x",
             g_IDT[0x80].Flags,
             g_IDT[0x80].BaseLow,
             g_IDT[0x80].BaseHigh);

    uint32_t idt_addr = g_IDT[0x80].BaseLow | (g_IDT[0x80].BaseHigh << 16);
    log_info("SYSCALL", "IDT[0x80] addr = 0x%x flags=0x%x", idt_addr, g_IDT[0x80].Flags);

    uint32_t isr_addr = (uint32_t)i686_ISR128;
    log_info("SYSCALL", "symbol i686_ISR128 = 0x%x", isr_addr);
}


void i686_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags)
{
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

void i686_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_Initialize()
{
    i686_IDT_Load(&g_IDTDescriptor);
}