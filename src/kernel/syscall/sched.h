#pragma once

#include <stdint.h>
#include <types.h>
#include "hal/process.h"
#include "hal/vfs.h"
#include "syscall_frame.h"
#include "kmalloc.h"
#include "paging.h"
#include "errno.h"
#include <arch/x86_64/msr.h>
#include <string.h>

extern volatile uint32_t g_poll_yields;

void sched_init(void);            /* call after x86_enable_fpu_sse() */
int proc_register(Process *p);
Process *proc_alloc(void);

void proc_reap(Process *z);

/* Make a NEW process runnable: its first run "returns" through ret_from_fork. */
void proc_start(Process *c, const struct syscall_frame *f);
static int index_of(Process *p) ;
static Process *pick_next(Process *cur);

static int count_runnable(void);
static void reap_orphans(void);
/* Call with IF=0 (always true inside a syscall). */
void schedule(void);
void sched_wait_yield(void);

__attribute__((noreturn)) void do_exit(int code);
long sys_wait4(int pid, uint64_t status_u, int options, uint64_t rusage_u);