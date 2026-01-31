// isr64.h
#pragma once
#include <stdint.h>

typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) CpuFrame64;

typedef struct {
    // lowest address: regs (in push order, low → high)
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t vector;
    uint64_t error;

    CpuFrame64 cpu;   // rip, cs, rflags, rsp, ss (highest)
} __attribute__((packed)) ISRFrame64;


typedef void (*ISR64Handler)(ISRFrame64* regs);

void x64_ISR_Initialize();
void x64_ISR_Handler(ISRFrame64* r);
void x64_ISR_RegisterHandler(int interrupt, ISR64Handler handler);
