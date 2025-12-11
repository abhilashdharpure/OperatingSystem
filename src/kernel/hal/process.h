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
    uint32_t *page_directory;   // <-- REQUIRED
    pid_t pid;
} Process;


Process *process_create(const char *name);
uintptr_t process_setup_stack(Process *p);
