#include "syscall_install.h"
#include "arch/x86_64/isr64.h"
#include "arch/x86_64/idt64.h"
#include "arch/x86_64/gdt.h"
#include "debug.h"

void x64_syscall_handler(ISRFrame64* regs);

void x64_syscall_install(void)
{
    x64_ISR_RegisterHandler(0x80, x64_syscall_handler);
    // dump_syscall_idt_entry();

    log_info("SYSCALL", "syscall vector 0x80 installed");
}
