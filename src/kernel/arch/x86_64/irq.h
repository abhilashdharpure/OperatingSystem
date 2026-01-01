#pragma once
#include <stdint.h>
#include "arch/x86_64/isr64.h"

#define PIC_REMAP_OFFSET 0x20

typedef void (*IRQHandler)(ISRFrame64* regs);

/* Initialize IRQ controller (PIC or APIC) */
void x64_IRQ_Initialize(void);

/* Register a handler for a specific IRQ line */
void x64_IRQ_RegisterHandler(int irq, IRQHandler handler);

void x64_IRQ_Unmask(int irq);

/* Send EOI to PIC/APIC */
void x64_IRQ_SendEndOfInterupt(int irq);

/* PIC driver interface (internal) */
struct PICDriver;
