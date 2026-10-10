#pragma once

#include <stdint.h>

/* Layout = what x64_syscall_entry pushes, lowest address first. */
struct syscall_frame
{
    uint64_t r9, r8, r10, rdx, rsi, rdi, r15, r14, r13, r12, rbp, rbx;
    uint64_t rip, cs, rflags, rsp, ss;       /* iretq frame */
};