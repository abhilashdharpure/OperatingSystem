#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>
#include <debug.h>

#define MODULE          "ISR"

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void i686_ISR_InitializeGates();

void i686_ISR_Initialize()
{
    i686_ISR_InitializeGates();
    log_info("Main", "i686_ISR_Initialize 2 ");

    // for (int i = 0; i < 256; i++)
    //     i686_IDT_EnableGate(i);

    for (int i = 0; i < 32; i++)
    {
        i686_IDT_EnableGate(i);
    }
    
    i686_IDT_EnableGate(0x80);
}

void __attribute__((cdecl)) i686_ISR_Handler(Registers* regs)
{
    if(regs->interrupt != 3 && regs->interrupt != 32)
    {
        log_info(MODULE, "ISR: interrupt=%d err=%x eip=%x cs=%x",
             regs->interrupt, regs->error, regs->eip, regs->cs);

    }

    if (regs->interrupt == 14)
    {
        uint32_t cr2;
        uint32_t cr3;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
        __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));

        log_critical("PF",
            "Page fault at eip=%x addr=%x err=%x cs=%x cr3=%x",
            regs->eip, cr2, regs->error, regs->cs, cr3);

        // Walk page tables of the *current* address space (CR3)
        uint32_t *pd = (uint32_t*)cr3;  // low RAM identity-mapped

        uint32_t pd_idx = (cr2 >> 22) & 0x3FF;
        uint32_t pt_idx = (cr2 >> 12) & 0x3FF;

        uint32_t pde = pd[pd_idx];
        log_critical("PF", "  PDE[%u] = 0x%x", pd_idx, pde);

        if (pde & 0x1) {
            uint32_t *pt = (uint32_t*)(pde & 0xFFFFF000); // PT phys, identity mapped
            uint32_t pte = pt[pt_idx];
            log_critical("PF", "  PTE[%u] = 0x%x", pt_idx, pte);
        } else {
            log_critical("PF", "  PDE not present");
        }

        i686_Panic(); // halt
    }

    if (regs->interrupt == 0x80) {
        log_info(MODULE, "i686_ISR_Handler interrupt (0x80) = %d!", regs->interrupt);

        i686_syscall_handler(regs);
        return;
    }

    if (g_ISRHandlers[regs->interrupt] != NULL)
    {
        g_ISRHandlers[regs->interrupt](regs);
    }
    else if (regs->interrupt >= 32)
    {
        log_error(MODULE, "Unhandled interrupt %d!", regs->interrupt);
    }
    else 
    {
        log_critical(MODULE, "Unhandled exception %d %s", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        log_critical(MODULE, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        log_critical(MODULE, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        log_critical(MODULE, "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);

        log_critical(MODULE, "KERNEL PANIC!");
        printf("KERNEL PANIC!");

        i686_Panic();
    }
}

void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    g_ISRHandlers[interrupt] = handler;
    i686_IDT_EnableGate(interrupt);
}
