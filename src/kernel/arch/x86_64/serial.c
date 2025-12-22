// serial.c
#include "serial.h"
#include <arch/x86_64/io.h>

void serial_init(void) {
    // your init_serial() contents
    outb(0x00, 0x3F9);
    outb(0x80, 0x3FB);
    outb(0x03, 0x3F8);
    outb(0x00, 0x3F9);
    outb(0x03, 0x3FB);
    outb(0xC7, 0x3FA);
    outb(0x0B, 0x3FC);
}

void serial_putc_asm(char c) {
    __asm__ volatile (
        "mov $0x3F8, %%dx\n\t"
        "mov %0, %%al\n\t"
        "out %%al, %%dx\n\t"
        :
        : "r"(c)
        : "dx", "al"
    );
}

void serial_putc(char c)
{
    // serial_putc_asm('C');

    while (!(inb(0x3F8 + 5) & 0x20))
        ;
    outb(0x3F8, c);
    // serial_putc_asm('D');
}

void serial_write(const char *s)
{
    while (*s)
    {
        serial_putc(*s);
        s++;
    }
}

void debug_print_cs(void)
{
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));

    const char hex[] = "0123456789ABCDEF";
    serial_putc_asm('[');
    serial_putc_asm(hex[(cs >> 12) & 0xF]);
    serial_putc_asm(hex[(cs >> 8) & 0xF]);
    serial_putc_asm(hex[(cs >> 4) & 0xF]);
    serial_putc_asm(hex[(cs >> 0) & 0xF]);
    serial_putc_asm(']');
}
