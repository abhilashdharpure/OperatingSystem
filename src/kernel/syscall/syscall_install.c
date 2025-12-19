#include "syscall_install.h"
#include "arch/i686/isr.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "debug.h"


// #include "arch/i686/isrs_gen.c"

void i686_syscall_handler(Registers* regs); // implemented below in syscall.c

void i686_syscall_install(void)
{
    i686_ISR_RegisterHandler(0x80, i686_syscall_handler);
    dump_syscall_idt_entry();

    log_info("SYSCALL", "syscall vector 0x80 installed");
}
