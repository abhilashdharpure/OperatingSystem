#pragma once

#include <stdint.h>
#include <arch/x86_64/isr64.h>

void x64_syscall_handler(ISRFrame64* regs);
