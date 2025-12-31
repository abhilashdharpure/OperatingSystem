#pragma once

#include <stdint.h>

typedef int pid_t;

typedef struct regs {
    uint64_t rip;  // 64-bit instruction pointer
    uint64_t rsp;  // 64-bit stack pointer
    // later: rflags, rax, rbx, rcx, rdx, rsi, rdi, rbp, etc.
} regs_t;

typedef struct Process {
    regs_t    regs;
    uint64_t *page_directory;   // PML4 kernel-virtual
    uint64_t  cr3;              // PML4 physical (for CR3)
    pid_t     pid;
} Process;

Process *process_create(const char *name);
uintptr_t process_setup_stack(Process *p);
