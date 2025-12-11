#pragma once
#include <stdint.h>

void switch_to_user_mode(uint32_t eip, uint32_t esp)
{
    // enter_user_mode(eip, esp);


        __asm__ volatile(
        "cli\n"                  // disable interrupts
        "movl %[stack], %%esp\n" // set user stack
        "pushl $0x23\n"          // user data segment selector (GDT)
        "pushl %[stack]\n"       // user stack pointer
        "pushf\n"                // push EFLAGS
        "pushl $0x1B\n"          // user code segment selector (GDT)
        "pushl %[entry]\n"       // user program entry
        "iret\n"                 // return to user mode
        :
        : [entry] "r"(eip), [stack] "r"(esp)
    );
}
