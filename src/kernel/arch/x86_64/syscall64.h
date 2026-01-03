// syscall64.h
#pragma once
#include <stdint.h>

// typedef struct syscall_regs {
//     uint64_t rax;
//     uint64_t rdi;
//     uint64_t rsi;
//     uint64_t rdx;
//     uint64_t r10;
//     uint64_t r8;
//     uint64_t r9;
// } syscall_regs_t;

typedef struct syscall_regs {
    uint64_t rcx;
    uint64_t r11;
    uint64_t r9;
    uint64_t r8;
    uint64_t r10;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
} syscall_regs_t;


void x64_SYSCALL_Initialize(void);
uint64_t syscall_dispatch(syscall_regs_t *r);
