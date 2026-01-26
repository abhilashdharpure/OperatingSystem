// isr64.c
#include "isr64.h"
#include "debug.h"
#include "hal/process.h"
#include "arch/x86_64/serial.h"
#include "arch/x86_64/irq.h"

ISR64Handler g_ISR64Handlers[256];
extern Process *current_process;

void x64_ISR_InitializeGates();

void x64_ISR_Initialize()
{
    x64_ISR_InitializeGates();
    log_info("Main", "x64_ISR_InitializeGates");

    // for (int i = 0; i < 256; i++)
    //     x64_IDT_EnableGate(i);

    for (int i = 0; i < 32; i++)
    {
        x64_IDT_EnableGate(i);
    }

    // for (int i = 32; i < 48; i++)
    // {
    //     x64_IDT_EnableGate(i);
    // }

    // Hardware IRQs (0x20–0x2F)
    for (int i = PIC_REMAP_OFFSET; i < PIC_REMAP_OFFSET + 16; i++)
    {
        x64_IDT_EnableGate(i);
    }
    
    x64_IDT_EnableGate(0x80);
}

static inline uint64_t read_cr2(void)
{
    uint64_t v; 
    __asm__ volatile("mov %%cr2, %0" : "=r"(v));
    return v;
}

void x64_ISR_Handler(ISRFrame64* r)
{
    if (r->vector == 1)
    {
        log_warning("EXC", "#DB ignored (TF was set)");
        return;
    }

    // // Temporary skiping PIT
    // if (r->vector != 32)
    // {
    //     log_error("ISR64", "x64_ISR_Handler Interupt r->vector = %u", r->vector);
    // }

    if (r->vector < 32)
    {
        if (r->vector == 13)
        {
            log_critical("GP",
                "#GP: error=%llx rip=%p cs=%llx ss=%llx rflags=%llx",
                r->error,
                (void*)r->cpu.rip,
                r->cpu.cs,
                r->cpu.ss,
                r->cpu.rflags
            );
        }

        if (r->vector == 14)
        {
            uint64_t cr2 = read_cr2();
            log_critical("PF", "Page fault: cr2=%p error=%llx", (void*)cr2, r->error);
        }

        if (r->vector == 6) {
            log_info("EXC", "UD at rip=0x%llx", (unsigned long long)r->cpu.rip);
            debug_dump_va_mapping(current_process->page_directory, r->cpu.rip);
            debug_dump_user_bytes(current_process, r->cpu.rip, 0x40);
        }


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
