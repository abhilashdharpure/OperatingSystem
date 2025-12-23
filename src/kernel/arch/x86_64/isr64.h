// isr64.h
#pragma once
#include <stdint.h>

typedef struct
{
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
    uint64_t vector;
} __attribute__((packed)) InterruptFrame64;


typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} __attribute__((packed)) CpuFrame64;

typedef struct {
    uint64_t vector;
    uint64_t error;

    /* saved registers */
    uint64_t rax, rcx, rdx, rbx;
    uint64_t rsi, rdi, rbp;
    uint64_t r8,  r9,  r10, r11, r12, r13, r14, r15;

    CpuFrame64 cpu;
} __attribute__((packed)) ISRFrame64;

typedef void (*ISR64Handler)(ISRFrame64* regs);


// void x64_ISR_CommonHandler(InterruptFrame64* frame);
void x64_ISR_Initialize();
void x64_ISR_Handler(ISRFrame64* r);
void x64_ISR_RegisterHandler(int interrupt, ISR64Handler handler);