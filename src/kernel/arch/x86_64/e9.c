#include "e9.h"
#include <arch/x86_64/io.h>

void e9_putc(char c)
{
    // i686_outb_local(0xE9, c);
    outb(0xE9, c);
}

