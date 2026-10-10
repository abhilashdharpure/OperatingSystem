#include "sched.h"
#include "hal/process.h"
#include "hal/vfs.h"
#include "syscall_frame.h"
#include "kmalloc.h"
#include "paging.h"
#include "debug.h"
#include "errno.h"
#include <arch/x86_64/msr.h>
#include <string.h>

#define MAX_PROCS   32
#define KSTACK_SIZE (32 * 1024)

extern uint64_t g_syscall_rsp0;
extern Process *current_process;
extern void switch_context(uint64_t *save_rsp, uint64_t new_rsp);
extern void ret_from_fork(void);
extern void x64_TSS_SetRsp0(uint64_t rsp0);   /* adapt: tss.rsp0 = v; */
extern void write_cr3(uint64_t pa);
extern void free_user_address_space(uint64_t pml4_pa);

static Process *g_procs[MAX_PROCS];
static int  g_next_pid = 1;
static int  g_have_boot = 0;
volatile uint32_t g_poll_yields;
static uint8_t g_fpu_tmpl_raw[512 + 16];

static inline uint8_t *al16(void *p) { return (uint8_t *)(((uint64_t)p + 15) & ~15ULL); }
uint8_t *proc_fpu(Process *p) { return al16(p->fpu_raw); }

void sched_init(void)            /* call after x86_enable_fpu_sse() */
{
    __asm__ volatile("fninit; fxsave (%0)" :: "r"(al16(g_fpu_tmpl_raw)) : "memory");
}

int proc_register(Process *p)
{
    for (int i = 0; i < MAX_PROCS; i++) {
        if (g_procs[i]) continue;
        p->pid = g_next_pid++;
        p->parent = NULL; p->exit_code = 0; p->auto_reap = false;
        memset(p->fds, 0, sizeof(p->fds));
        memcpy(proc_fpu(p), al16(g_fpu_tmpl_raw), 512);

        if (!g_have_boot) {                 /* first process (init) uses the boot stack */
            g_have_boot = 1;
            p->kstack_base = NULL;
            p->kstack_top  = g_syscall_rsp0;
            p->state = PROC_RUNNABLE;
        } else {
            p->kstack_base = kmalloc(KSTACK_SIZE);
            if (!p->kstack_base) return -ENOMEM;
            p->kstack_top = ((uint64_t)p->kstack_base + KSTACK_SIZE) & ~0xFULL;
            p->state = PROC_NEW;            /* not schedulable until proc_start */
        }
        g_procs[i] = p;
        return 0;
    }
    return -EAGAIN;
}

Process *proc_alloc(void)
{
    Process *p = process_create("user");    /* adapt */
    if (!p) return NULL;
    if (proc_register(p) < 0) { kfree(p); return NULL; }
    return p;
}

void proc_reap(Process *z)
{
    for (int i = 0; i < MAX_PROCS; i++) if (g_procs[i] == z) g_procs[i] = NULL;
    if (z->cr3)         free_user_address_space(z->cr3);
    if (z->kstack_base) kfree(z->kstack_base);
    kfree(z);                               /* adapt to how process_create allocates */
}

/* Make a NEW process runnable: its first run "returns" through ret_from_fork. */
void proc_start(Process *c, const struct syscall_frame *f)
{
    struct syscall_frame *kf = (struct syscall_frame *)(c->kstack_top - sizeof(*kf));
    *kf = *f;
    uint64_t *sp = (uint64_t *)kf;
    *--sp = (uint64_t)ret_from_fork;        /* popped by switch_context's ret */
    for (int i = 0; i < 6; i++) *--sp = 0;  /* rbx rbp r12 r13 r14 r15 */
    c->ksp   = (uint64_t)sp;
    c->state = PROC_RUNNABLE;
}

static int index_of(Process *p) { for (int i = 0; i < MAX_PROCS; i++) if (g_procs[i] == p) return i; return 0; }

static Process *pick_next(Process *cur)
{
    int s = index_of(cur);
    for (int k = 1; k <= MAX_PROCS; k++) {
        Process *p = g_procs[(s + k) % MAX_PROCS];
        if (p && p != cur && p->state == PROC_RUNNABLE) return p;
    }
    return NULL;
}

static int count_runnable(void)
{
    int n = 0;
    for (int i = 0; i < MAX_PROCS; i++) if (g_procs[i] && g_procs[i]->state == PROC_RUNNABLE) n++;
    return n;
}

static void reap_orphans(void)
{
    for (int i = 0; i < MAX_PROCS; i++) {
        Process *p = g_procs[i];
        if (p && p != current_process && p->state == PROC_ZOMBIE && (!p->parent || p->auto_reap))
            proc_reap(p);
    }
}

/* Call with IF=0 (always true inside a syscall). */
void schedule(void)
{
    reap_orphans();
    Process *prev = current_process;
    Process *next = pick_next(prev);
    if (!next) return;

    __asm__ volatile("fxsave (%0)" :: "r"(proc_fpu(prev)) : "memory");
    __asm__ volatile("fxrstor (%0)" :: "r"(proc_fpu(next)) : "memory");

    current_process = next;
    g_syscall_rsp0  = next->kstack_top;
    x64_TSS_SetRsp0(next->kstack_top);
    if (next->cr3 != prev->cr3) write_cr3(next->cr3);
    wrmsr(MSR_FS_BASE, next->fs_base);

    switch_context(&prev->ksp, next->ksp);  /* returns here when prev runs again */
}

void sched_wait_yield(void)
{
    int n = count_runnable();
    if (n > 1 && ++g_poll_yields <= (uint32_t)n) { schedule(); return; }
    g_poll_yields = 0;
    __asm__ volatile("sti; hlt; cli");      /* everyone is waiting: sleep until an IRQ */
}

__attribute__((noreturn)) void do_exit(int code)
{
    Process *p = current_process;
    for (int i = 0; i < PROC_MAX_FDS; i++) {
        struct file *f = p->fds[i];
        if (f && f != (struct file *)1) { p->fds[i] = NULL; VFS_PutFile(f); } /* sockets give EOF to peers */
    }
    for (int i = 0; i < MAX_PROCS; i++)
        if (g_procs[i] && g_procs[i]->parent == p) g_procs[i]->parent = NULL;

    p->exit_code = code;
    p->state = PROC_ZOMBIE;
    for (;;) { schedule(); __asm__ volatile("sti; hlt; cli"); }
}

long sys_wait4(int pid, uint64_t status_u, int options, uint64_t rusage_u)
{
    (void)rusage_u;
    Process *me = current_process;
    for (;;) {
        int have = 0;
        for (int i = 0; i < MAX_PROCS; i++) {
            Process *q = g_procs[i];
            if (!q || q->parent != me || (pid > 0 && q->pid != pid)) continue;
            have = 1;
            if (q->state == PROC_ZOMBIE) {
                int st = (q->exit_code & 0xff) << 8, cpid = q->pid;
                if (status_u) copy_to_user(status_u, &st, sizeof(st));
                proc_reap(q);
                return cpid;
            }
        }
        if (!have)          return -ECHILD;
        if (options & 1)    return 0;               /* WNOHANG */
        sched_wait_yield();
    }
}