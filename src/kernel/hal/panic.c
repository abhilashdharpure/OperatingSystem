// panic.c
#include <debug.h>
void panic(const char *msg)
{
    log_critical("KERNEL", "ERROR PANIC: %s\n", msg);
    boot_fill(0x00B00000);
    for (;;) { __asm__ volatile("hlt"); }
}



