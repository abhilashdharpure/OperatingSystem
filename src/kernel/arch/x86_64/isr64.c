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



void dump_iret_frame(uint64_t *sp)
{
    log_info("IRET", "RIP=%lx CS=%lx RFLAGS=%lx RSP=%lx SS=%lx",
        sp[0], sp[1], sp[2], sp[3], sp[4]);
}


void x64_ISR_Handler(ISRFrame64* r)
{
    uint8_t vec = (uint8_t)r->vector; 
    if (vec == 1)
    {
        log_warning("EXC", "#DB ignored (TF was set)");
        return;
    }

    // Temporary skiping PIT
    if (vec!= 32)
    {
        //log_error("ISR64", "x64_ISR_Handler Interupt r->vector = %u", vec);
    }

    if (vec < 32)
    {

        if (vec == 6)
        {
            log_info("EXC", "UD at rip=0x%llx", (unsigned long long)r->cpu.rip);
            debug_dump_va_mapping(current_process->page_directory, r->cpu.rip);
            debug_dump_user_bytes(current_process, r->cpu.rip, 0x40);
        }
        else if (vec == 7)
        {
            uint64_t cr0_before, cr0_after;

            __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0_before));
            uint64_t rip = r->cpu.rip;
            uint64_t cs  = r->cpu.cs;

            log_critical("NM",
                "#NM at rip=0x%llx cs=0x%llx CR0=0x%llx",
                (unsigned long long)rip,
                (unsigned long long)cs,
                (unsigned long long)cr0_before);

            // optional: dump mapping + bytes at RIP
            debug_dump_va_mapping(current_process->page_directory, rip);
            debug_dump_user_bytes(current_process, rip, 0x40);

            // keep your CR0 fixup (even though EM/TS are 0)
            uint64_t cr0 = cr0_before;
            cr0 &= ~(1ULL << 3); // TS = 0
            cr0 &= ~(1ULL << 2); // EM = 0
            __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");
            __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0_after));
            log_info("NM", "CR0 after =0x%llx", (unsigned long long)cr0_after);

            return;
        }



        // else if (vec == 7)
        // { // #NM
        //     uint64_t cr0;
        //     __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
        //     cr0 &= ~(1ULL << 3); // clear TS
        //     __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");
        //     panic();
        // }
        else if (vec == 13) {
            log_critical("GP",
                "#GP: error=%llx rip=%p cs=%llx ss=%llx rflags=%llx",
                r->error,
                (void*)r->cpu.rip,
                r->cpu.cs,
                r->cpu.ss,
                r->cpu.rflags
            );

            // r is at top of stack; IRET frame is just below ISRFrame64
            uint64_t *iret = (uint64_t *)((uint8_t *)r + sizeof(ISRFrame64));
            dump_iret_frame(iret);
        }
        else if (vec == 14)
        {
            uint64_t cr2 = read_cr2();
            log_critical("PF", "Page fault: cr2=%p error=%llx", (void*)cr2, r->error);
            debug_dump_va_mapping(current_process->page_directory, r->cpu.rip);
            debug_dump_user_bytes(current_process, r->cpu.rip, 0x40);

        }

        log_critical("EXC",
            "vec=%llu rip=%p cs=%llx rflags=%llx",
            vec,
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

    if (g_ISR64Handlers[vec]) {
        g_ISR64Handlers[vec](r);
    }
}

void x64_ISR_RegisterHandler(int interrupt, ISR64Handler handler)
{
    g_ISR64Handlers[interrupt] = handler;
    x64_IDT_EnableGate(interrupt);
}
