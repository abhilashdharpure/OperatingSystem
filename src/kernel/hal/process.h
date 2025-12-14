#pragma once

#include <stdint.h>

typedef int pid_t;

typedef struct regs {
    uintptr_t eip;
    uintptr_t esp;
    // other registers
} regs_t;

typedef struct Process {
    regs_t regs;
    uint32_t *page_directory;   // kernel-virtual PD
    uint32_t cr3;               // physical PD (for CR3)
    pid_t pid;
} Process;



Process *process_create(const char *name);
uintptr_t process_setup_stack(Process *p);
