#pragma once

#include <arch/x86_64/isr64.h>

void x64_syscall_handler(ISRFrame64* regs);
void x64_syscall_install(void);
