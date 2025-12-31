#include "irq.h"
#include "pic.h"
#include "i8259.h"
#include "arch/x86_64/isr64.h"
#include <stddef.h>
#include <util/arrays.h>
#include <debug.h>
#include "arch/x86_64/events/input_event.h"

#define MODULE "PIC"

IRQHandler g_IRQHandlers[16] = {0};
const PICDriver* g_IRQ_Driver = NULL;

void x64_IRQ_Handler(ISRFrame64* r)
{
    int vector = r->vector;
    int irq = vector - PIC_REMAP_OFFSET;

    if (irq < 0 || irq >= 16) {
        log_error("IRQ", "Invalid IRQ vector %u", vector);
        return;
    }

    if (g_IRQHandlers[irq]) {
        g_IRQHandlers[irq](r);
    }

    if (irq == 7 || irq == 15) {
        x64_IRQ_SendEndOfInterupt(irq);
        return;
    }
    x64_IRQ_SendEndOfInterupt(irq);
}

/* Initialize PIC and hook IRQs to IDT */
void x64_IRQ_Initialize(void)
{
    const PICDriver* drivers[] = {
        i8259_GetDriver(),
    };


    /* find first available PIC driver */
    for (int i = 0; i < SIZE(drivers); i++) {
        if (drivers[i]->Probe()) {
            g_IRQ_Driver = drivers[i];
            break;
        }
    }

    if (g_IRQ_Driver == NULL) {
        log_warning(MODULE, "No PIC found!");
        return;
    }

    log_info(MODULE, "Found %s PIC.", g_IRQ_Driver->Name);
    g_IRQ_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);
    

    /* Register ISR64 handlers for IRQ vectors (0x20–0x2F) */
    for (int i = 0; i < 16; i++) {
        x64_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, x64_IRQ_Handler);
    }

    /* Unmask common hardware IRQs */
    g_IRQ_Driver->Unmask(HardwareIRQNo_PIT_Timer);
    g_IRQ_Driver->Unmask(HardwareIRQNo_Keyboard);
    g_IRQ_Driver->Unmask(HardwareIRQNo_Cascade);
    g_IRQ_Driver->Unmask(HardwareIRQNo_Mouse);
    

    // /* Enable interrupts in RFLAGS */
    // __asm__ volatile("sti");
}

/* Register a specific IRQ handler */
void x64_IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    if (irq < 0 || irq >= 16)
        return;

    // g_IRQHandlers[irq] = handler;

    // int vector = PIC_REMAP_OFFSET + irq;
    // x64_IDT_EnableGate(vector);

    int vector = PIC_REMAP_OFFSET + irq;
    g_IRQHandlers[irq] = handler;
    x64_IDT_EnableGate(vector);
}

void x64_IRQ_Unmask(int irq)
{
    g_IRQ_Driver->Unmask(irq);
}


/* Send EOI to PIC/APIC */
void x64_IRQ_SendEndOfInterupt(int irq)
{
    g_IRQ_Driver->SendEndOfInterrupt(irq);
}


// #include "irq.h"
// #include "pic.h"
// #include "i8259.h"
// #include "io.h"
// #include <stddef.h>
// #include <util/arrays.h>
// #include "stdio.h"
// #include <debug.h>
// #include "arch/x86_64/events/input_event.h"

// #define PIC_REMAP_OFFSET        0x20
// #define MODULE                  "PIC"

// IRQHandler g_IRQHandlers[16];
// static const PICDriver* g_Driver = NULL;

// void i686_IRQ_Handler(Registers* regs)
// {
//     int irq = regs->interrupt - PIC_REMAP_OFFSET;

//     if (irq >= 0 && irq < 16 && g_IRQHandlers[irq] != NULL)
//     {
//         g_IRQHandlers[irq](regs);
//     } 
//     else {
//         log_warning(MODULE, "Unhandled IRQ %d...", irq);
//     }

//     // send EOI
//     g_Driver->SendEndOfInterrupt(irq);
// }

// void i686_IRQ_Initialize()
// {
//     const PICDriver* drivers[] = {
//         i8259_GetDriver(),
//     };

//     for (int i = 0; i < SIZE(drivers); i++) {
//         if (drivers[i]->Probe()) {
//             g_Driver = drivers[i];
//         }
//     }

//     if (g_Driver == NULL) {
//         log_warning(MODULE, "No PIC found!");
//         return;
//     }

//     log_info(MODULE, "Found %s PIC.", g_Driver->Name);
//     g_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

//     // register ISR handlers for each of the 16 irq lines
//     for (int i = 0; i < 16; i++)
//         i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);

//     g_Driver->Unmask(HardwareIRQNo_PIT_Timer);
//     g_Driver->Unmask(HardwareIRQNo_Keyboard);
//     g_Driver->Unmask(HardwareIRQNo_Cascade);
//     g_Driver->Unmask(HardwareIRQNo_Mouse);

//     // enable interrupts
//     enable_interrupts();

// }

// void i686_IRQ_RegisterHandler(int irq, IRQHandler handler)
// {
//     g_IRQHandlers[irq] = handler;
// }

// void i686_IRQ_SendEndOfInterupt(int irq)
// {
//     // send EOI
//     g_Driver->SendEndOfInterrupt(irq);
// }