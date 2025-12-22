// serial.h
#pragma once
#include <stdint.h>


void serial_putc_asm(char c);
void serial_init(void);
void serial_putc(char c);
void serial_write(const char *s);
void debug_print_cs(void);
