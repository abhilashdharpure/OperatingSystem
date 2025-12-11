#pragma once
#include <stdint.h>

/* install TSS: provide kernel ring0 stack pointer (esp0) */
void i686_TSS_Install(uint32_t kernel_esp0);

/* selector value for the TSS (you can use to debug) */
uint16_t i686_TSS_Selector(void);
