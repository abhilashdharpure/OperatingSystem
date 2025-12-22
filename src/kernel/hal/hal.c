#include "hal.h"
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/isr.h>
#include <arch/x86_64/irq.h>
#include <arch/x86_64/vga_text.h>
#include <arch/x86_64/ps2.h>
#include <arch/x86_64/pit.h>
#include "tss.h"
#include "syscall/syscall_install.h"
#include "debug.h"

// // Allocate a 16 KB kernel stack
// static uint8_t kernel_stack[16384];

// // Expose pointer for TSS
// uintptr_t kernel_stack_top = (uintptr_t)(kernel_stack + sizeof(kernel_stack));

// // Use a stack in a region that is identity-mapped and reserved
// static uint8_t kernel_stack[16384] __attribute__((aligned(16)));
// uintptr_t kernel_stack_top = (uintptr_t)(kernel_stack + sizeof(kernel_stack));

#define KERNEL_STACK_PHYS 0x00800000  // for example
#define KERNEL_STACK_SIZE 0x4000      // 16 KiB

uintptr_t _kernel_stack_bottom = KERNEL_STACK_PHYS;
uintptr_t _kernel_stack_top    = KERNEL_STACK_PHYS + KERNEL_STACK_SIZE;


// extern uint8_t _kernel_stack_top;
// extern uint8_t _kernel_stack_bottom;


/* kernel test - place after i686_syscall_install() */
static void test_int80_kernel(void)
{
    log_info("TEST", "kernel: invoking int $0x80");
    __asm__ volatile("int $0x80");
}


void HAL_Initialize()
{
    // log_info("Main", "Kernel HAL_Initialize");

    // log_info("STACK", "kernel stack bottom = %p", &_kernel_stack_bottom);
    // log_info("STACK", "kernel stack top    = %p", &_kernel_stack_top);

    __asm__ volatile ("cli");
    gdt_init();
    // i686_GDT_Initialize();
    /* kernel_stack_top: top of kernel stack (virtual address) used for ring0 on interrupts */
    i686_TSS_Install(_kernel_stack_top);
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();

    InitializeDevNull();
    i686_syscall_install();
    // test_int80_kernel();
    __asm__ volatile ("sti");
    // init keyboard
    ps2_init();  

    // init timer
    uint32_t frequency = 1000;
    pit_init(frequency);

    VGA_clrscr();
}
