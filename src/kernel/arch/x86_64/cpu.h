#pragma once

#include <stdint.h>

void write_cr3(uint64_t value);

static inline uint64_t read_cr3(void)
{
    uint64_t val; 
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}