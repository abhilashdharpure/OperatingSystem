#include "uprintf.h"
#include <stdarg.h>
#include <unistd.h>

static void putc_internal(char c) {
    write(1, &c, 1);
}

static void print_string(const char *s) {
    if (!s) {
        print_string("(null)");
        return;
    }
    while (*s) putc_internal(*s++);
}


static void print_decimal_long(long v) {
    char buf[32];
    int i = 0;

    if (v == 0) {
        putc_internal('0');
        return;
    }

    if (v < 0) {
        putc_internal('-');
        v = -v;
    }

    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }

    while (i--) putc_internal(buf[i]);
}

static void print_decimal_unsigned(unsigned long v) {
    char buf[32];
    int i = 0;

    if (v == 0) {
        putc_internal('0');
        return;
    }

    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }

    while (i--) putc_internal(buf[i]);
}

static void print_hex(unsigned long v) {
    const char *hex = "0123456789abcdef";
    char buf[32];
    int i = 0;

    if (v == 0) {
        putc_internal('0');
        return;
    }

    while (v > 0) {
        buf[i++] = hex[v & 0xF];
        v >>= 4;
    }

    while (i--) putc_internal(buf[i]);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {

        if (*p != '%') {
            putc_internal(*p);
            continue;
        }

        p++;  // skip '%'

        switch (*p) {

        case 's': {
            const char *s = va_arg(args, const char*);
            if (!s) print_string("(null)");
            else    print_string(s);
            break;
        }

        case 'c': {
            char c = (char)va_arg(args, int);
            putc_internal(c);
            break;
        }

        case 'd': {   // int
            int v = va_arg(args, int);
            print_decimal_long((long)v);
            break;
        }

        case 'u': {   // unsigned int
            unsigned int v = va_arg(args, unsigned int);
            print_decimal_unsigned((unsigned long)v);
            break;
        }

        case 'l': {   // long or unsigned long
            p++;       // look at next char: d / u / x
            if (*p == 'd') {
                long v = va_arg(args, long);
                print_decimal_long(v);
            } else if (*p == 'u') {
                unsigned long v = va_arg(args, unsigned long);
                print_decimal_unsigned(v);
            } else if (*p == 'x') {
                unsigned long v = va_arg(args, unsigned long);
                print_hex(v);
            } else {
                putc_internal('%');
                putc_internal('l');
                putc_internal(*p);
            }
            break;
        }

        case 'x': {   // hex int
            unsigned int v = va_arg(args, unsigned int);
            print_hex((unsigned long)v);
            break;
        }

        case '%':
            putc_internal('%');
            break;

        default:
            // Unknown format specifier: print literally
            putc_internal('%');
            putc_internal(*p);
            break;
        }
    }

    va_end(args);
}
