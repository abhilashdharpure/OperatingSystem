// syscall64.h
#pragma once
#include <stdint.h>

typedef struct syscall_regs {
    uint64_t r11;  // top of stack (last pushed)
    uint64_t rcx;
    uint64_t r9;
    uint64_t r8;
    uint64_t r10;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;  // bottom of saved area (first pushed)
} syscall_regs_t;

void     x64_SYSCALL_Initialize(void);

uint64_t syscall_dispatch(uint64_t nr,
                          uint64_t a0,
                          uint64_t a1,
                          uint64_t a2,
                          uint64_t a3,
                          uint64_t a4,
                          uint64_t a5);

void debug_syscall_regs(uint64_t nr,
                        uint64_t a0,
                        uint64_t a1,
                        uint64_t a2,
                        uint64_t a3,
                        uint64_t a4,
                        uint64_t a5,
                        uint64_t rsp_before_call);