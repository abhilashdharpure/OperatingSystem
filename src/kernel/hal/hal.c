#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>
#include <arch/i686/ps2.h>
#include <arch/i686/pit.h>
#include "tss.h"
#include "syscall/syscall_install.h"
#include "debug.h"

// // Allocate a 16 KB kernel stack
// static uint8_t kernel_stack[16384];

// // Expose pointer for TSS
// uintptr_t kernel_stack_top = (uintptr_t)(kernel_stack + sizeof(kernel_stack));

// Use a stack in a region that is identity-mapped and reserved
static uint8_t kernel_stack[16384] __attribute__((aligned(16)));
uintptr_t kernel_stack_top = (uintptr_t)(kernel_stack + sizeof(kernel_stack));

/* kernel test - place after i686_syscall_install() */
static void test_int80_kernel(void)
{
    log_info("TEST", "kernel: invoking int $0x80");
    __asm__ volatile("int $0x80");
    log_info("TEST", "kernel: returned from int $0x80");
}


void HAL_Initialize()
{
    VGA_clrscr();
    i686_GDT_Initialize();
    /* kernel_stack_top: top of kernel stack (virtual address) used for ring0 on interrupts */
    i686_TSS_Install(kernel_stack_top);
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    InitializeDevNull();
    i686_syscall_install();
    test_int80_kernel();

    // init keyboard
    ps2_init();  

    // init timer
    uint32_t frequency = 1000;
    pit_init(frequency);
}
