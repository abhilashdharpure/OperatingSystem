// isr64.c
#include "isr64.h"
#include "debug.h"
#include "arch/x86_64/serial.h"

ISR64Handler g_ISR64Handlers[256];

void x64_ISR_InitializeGates();

void x64_ISR_Initialize()
{
    x64_ISR_InitializeGates();
    log_info("Main", "i686_ISR_Initialize 2 ");

    // for (int i = 0; i < 256; i++)
    //     x64_IDT_EnableGate(i);

    for (int i = 0; i < 32; i++)
    {
        x64_IDT_EnableGate(i);
    }

    for (int i = 32; i < 48; i++)
    {
        x64_IDT_EnableGate(i);
    }
    
    x64_IDT_EnableGate(0x80);
}

void x64_ISR_Handler(ISRFrame64* r)
{

    if (r->vector == 1)
    {
        log_warning("EXC", "#DB ignored (TF was set)");
        return;
    }

    log_error("ISR64", "x64_ISR_Handler Interupt r->vector = %u", r->vector);
    if (r->vector < 32)
    {
        log_critical("EXC",
            "vec=%llu rip=%p cs=%llx rflags=%llx",
            r->vector,
            (void*)r->cpu.rip,
            r->cpu.cs,
            r->cpu.rflags
        );
        panic();
    }

    if (r->vector == 0x80) {
        x64_syscall_handler(r);
        return;
    }

    if (g_ISR64Handlers[r->vector]) {
        g_ISR64Handlers[r->vector](r);
    }
}

void x64_ISR_RegisterHandler(int interrupt, ISR64Handler handler)
{
    g_ISR64Handlers[interrupt] = handler;
    x64_IDT_EnableGate(interrupt);
}
