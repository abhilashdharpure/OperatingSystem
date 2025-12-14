#include "e9.h"
#include <arch/i686/io.h>

// static inline void i686_outb_local(uint16_t port, uint8_t val)
// {
//     __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
// }

void e9_putc(char c)
{
    // i686_outb_local(0xE9, c);
    i686_outb(0xE9, c);
}

