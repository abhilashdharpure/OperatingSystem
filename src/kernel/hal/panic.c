// panic.c
#include <debug.h>
void panic(const char *msg)
{
    log_critical("KERNEL", "ERROR PANIC: %s\n", msg);
    for (;;) { __asm__ volatile("hlt"); }
}



