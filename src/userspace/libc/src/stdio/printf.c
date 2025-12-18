#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

static void putc_internal(char c) {
    write(1, &c, 1);
}

void putchar(char c) {
    putc_internal(c);
}

void puts(const char *s) {
    while (*s) putc_internal(*s++);
    putc_internal('\n');
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            putc_internal(*p);
            continue;
        }

        p++;
        if (*p == 's') {
            const char *s = va_arg(args, const char*);
            while (*s) putc_internal(*s++);
        } else if (*p == 'd') {
            int v = va_arg(args, int);
            char buf[32];
            int i = 0;
            if (v == 0) {
                putc_internal('0');
                continue;
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
        } else if (*p == 'c') {
            char c = (char)va_arg(args, int);
            putc_internal(c);
        } else if (*p == '%') {
            putc_internal('%');
        }
    }

    va_end(args);
}
