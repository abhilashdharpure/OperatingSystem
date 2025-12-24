#include "hal.h"
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt64.h>
#include <arch/x86_64/isr64.h>
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

// uintptr_t _kernel_stack_bottom = KERNEL_STACK_PHYS;
// uintptr_t _kernel_stack_top    = KERNEL_STACK_PHYS + KERNEL_STACK_SIZE;


// extern uint8_t _kernel_stack_top;
// extern uint8_t _kernel_stack_bottom;

extern uint8_t _kernel_stack_bottom[];
extern uint8_t _kernel_stack_top[];

/* kernel test - place after x64_syscall_install() */
static void test_int80_kernel(void)
{
    log_info("TEST", "kernel: invoking int $0x80");
    __asm__ volatile("int $0x80");
}

static inline void clear_trap_flag(void)
{
    __asm__ volatile (
        "pushfq\n"
        "andq $~0x100, (%%rsp)\n"  // clear TF (bit 8)
        "popfq\n"
        :
        :
        : "memory"
    );
}


void HAL_Initialize()
{
    log_info("HAL", "Kernel HAL_Initialize");

    log_info("STACK", "kernel stack bottom = %p", _kernel_stack_bottom);
    log_info("STACK", "kernel stack top    = %p", _kernel_stack_top);


    // __asm__ volatile ("cli");
    gdt_init();
    log_info("HAL", "After gdt_init");
    // i686_GDT_Initialize();
    /* kernel_stack_top: top of kernel stack (virtual address) used for ring0 on interrupts */
    x64_TSS_Install((uintptr_t)_kernel_stack_top);
    log_info("HAL", "After x64_TSS_Install");

    x64_IDT_Initialize();
    log_info("HAL", "After x64_IDT_Initialize");

    // i686_ISR_Initialize();
    x64_ISR_Initialize();
    log_info("HAL", "After x64_ISR_Initialize");

    x64_IRQ_Initialize();
    log_info("HAL", "After x64_IRQ_Initialize");

    InitializeDevNull();
    log_info("HAL", "After InitializeDevNull");

    x64_syscall_install();
    log_info("HAL", "After x64_syscall_install");

    // test_int80_kernel();
    // __asm__ volatile ("sti");
    // init keyboard
    ps2_init();  
    log_info("HAL", "After ps2_init");

    // init timer
    uint32_t frequency = 1000;
    pit_init(frequency);
    log_info("HAL", "After pit_init");

    clear_trap_flag();
    __asm__ volatile("sti");
    log_info("HAL", "Interrupts enabled");
    
    VGA_clrscr();
    log_info("HAL", "After VGA_clrscr");

}
