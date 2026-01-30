#include "hal.h"
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt64.h>
#include <arch/x86_64/isr64.h>
#include <arch/x86_64/irq.h>
#include <arch/x86_64/vga_text.h>
#include <arch/x86_64/ps2.h>
#include <arch/x86_64/pit.h>
#include <arch/x86_64/io.h>
#include "tss.h"
#include "syscall/syscall_install.h"
#include "debug.h"
#include "arch/x86_64/paging_bootstrap.h"

#define KERNEL_STACK_PHYS 0x00800000  // for example
#define KERNEL_STACK_SIZE 0x4000      // 16 KiB

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

extern GDTEntry g_GDT[];
void gdt_debug_dump(void)
{
    uint64_t *raw = (uint64_t *)&g_GDT[0];
    for (int i = 0; i < 7; ++i)
    {
        log_info("GDT", "entry[%d] = 0x%llx",i, (unsigned long long)raw[i]);
    }
}

void x86_enable_fpu_sse(void)
{
    uint64_t cr0, cr4;

    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));

    // CR0: clear EM, set MP, clear TS
    cr0 &= ~(1ULL << 2);  // EM = 0 (no emulation)
    cr0 |=  (1ULL << 1);  // MP = 1 (monitor coprocessor)
    cr0 &= ~(1ULL << 3);  // TS = 0 (task switched clear)

    // CR4: enable SSE
    cr4 |= (1ULL << 9);   // OSFXSR
    cr4 |= (1ULL << 10);  // OSXMMEXCPT

    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");
    __asm__ volatile ("mov %0, %%cr4" :: "r"(cr4) : "memory");
}

void HAL_Initialize()
{
    log_info("HAL", "Kernel HAL_Initialize");

    log_info("STACK", "kernel stack bottom = %p", _kernel_stack_bottom);
    log_info("STACK", "kernel stack top    = %p", _kernel_stack_top);

    gdt_init();
    log_info("HAL", "After gdt_init");
    gdt_debug_dump();

    x64_TSS_Install((uintptr_t)_kernel_stack_top);
    log_info("HAL", "After x64_TSS_Install");

    x64_IDT_Initialize();
    log_info("HAL", "After x64_IDT_Initialize");

    paging_init_long_mode_globals();
    log_info("HAL", "After paging_init_long_mode_globals");

    x64_SYSCALL_Initialize();
    log_info("HAL", "After x64_SYSCALL_Initialize");

    x64_ISR_Initialize();
    log_info("HAL", "After x64_ISR_Initialize");

    x64_IRQ_Initialize();
    log_info("HAL", "After x64_IRQ_Initialize");

    InitializeDevNull();
    log_info("HAL", "After InitializeDevNull");

    x64_syscall_install();
    log_info("HAL", "After x64_syscall_install");

    // Init PIT **before enabling interrupts**
    uint32_t frequency = 1000;
    pit_init(frequency);
    log_info("HAL", "After pit_init");

    // Clear trap flag and enable interrupts
    clear_trap_flag();
    // enable_interrupts();

    // __asm__ volatile("sti");
    // log_info("HAL", "Interrupts enabled");

    x86_enable_fpu_sse();
    log_info("HAL", "After x86_enable_fpu_sse");

    uint64_t t0 = pit_get_ticks();
    for (volatile int i = 0; i < 1000000; i++);
    uint64_t t1 = pit_get_ticks();
    log_info("PIT", "Ticks: %llu -> %llu", t0, t1);

    // Init keyboard/mouse
    ps2_init();
    log_info("HAL", "After ps2_init");

    VGA_clrscr();
    log_info("HAL", "After VGA_clrscr");
}