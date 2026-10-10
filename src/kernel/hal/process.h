#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef int pid_t;
#define PROC_MAX_FDS 128            /* must be >= MAX_OPEN_FILES in vfs.h */

typedef struct regs {
    uint64_t rip;     // 0
    uint64_t rsp;     // 8

    uint64_t rflags;  // 16
    uint64_t cs;      // 24
    uint64_t ss;      // 32
    // later: rax, rbx, rcx, ...
} regs_t;


typedef enum
{
    PROC_UNUSED = 0,
    PROC_NEW,
    PROC_RUNNABLE,
    PROC_ZOMBIE
} proc_state_t;


typedef struct Process {
    regs_t      regs;
    uint64_t*   page_directory;   // PML4 kernel-virtual
    uint64_t    cr3;              // PML4 physical (for CR3)
    pid_t       pid;
    uint64_t    mmap_base;  // next free VA for mmap

    uint64_t    brk_start;  // heap region start
    uint64_t    brk_end;    // heap region limit (max)
    uint64_t    brk_cur;    // current program break
    uint64_t    brk_end_limit;

    uint64_t    fs_base;
    uint64_t    gs_base;

    uintptr_t kernel_stack_base;
    uintptr_t kernel_stack_top;

    // NEW: user stack metadata
    uint64_t    stack_base;             // lowest mapped stack VA
    uint64_t    stack_guard_page;       // one page below stack_base
    uint64_t    stack_soft_limit_bottom;// soft limit (like RLIMIT_STACK)

    proc_state_t     state;
    struct Process  *parent;
    int              exit_code;
    bool             auto_reap;      /* reaped without wait4 (SPAWN_DETACH) */
    uint64_t         ksp;            /* saved kernel rsp (switch_context) */
    uint64_t         kstack_top;     /* 16-byte aligned */
    void            *kstack_base;    /* NULL for the boot stack */
    struct file     *fds[PROC_MAX_FDS];
    uint8_t          fpu_raw[512 + 16];   /* fxsave area, aligned manually */
} Process;


extern Process *current_process;

Process *process_create(const char *name);
