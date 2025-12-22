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

void x64_ISR_CommonHandler(InterruptFrame64* frame);
