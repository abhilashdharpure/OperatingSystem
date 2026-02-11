#pragma once

#include <stddef.h>
#include <stdarg.h>

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list args);
int puts(const char *s);
int putchar(int c);
