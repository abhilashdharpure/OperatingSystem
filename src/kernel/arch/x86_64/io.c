#include "io.h"

#define UNUSED_PORT         0x80

void x86_iowait()
{
    outb(UNUSED_PORT, 0);
}
// uint16_t __attribute__((cdecl)) inw(uint16_t port) {
//     uint16_t val;
//     __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
//     return val;
// }

// void __attribute__((cdecl)) outw(uint16_t port, uint16_t value) {
//     __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
// }