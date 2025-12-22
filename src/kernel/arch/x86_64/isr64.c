// isr64.c
#include "isr64.h"
#include "debug.h"
#include "arch/x86_64/serial.h"

void x64_ISR_CommonHandler(InterruptFrame64* frame)
{
    serial_putc_asm('!'); // mark that we hit the ISR

    // log_error("ISR64",
    //           "Trap: RIP=%p CS=0x%llx RFLAGS=0x%llx RSP=%p SS=0x%llx vec=%llu",
    //           (void*)frame->rip,
    //           (unsigned long long)frame->cs,
    //           (unsigned long long)frame->rflags,
    //           (void*)frame->rsp,
    //           (unsigned long long)frame->ss,
    //           (unsigned long long)frame->vector);


    for (;;) {
        __asm__ volatile("hlt");
    }
}
