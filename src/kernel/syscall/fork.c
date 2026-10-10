#include "fork.h"
#include "hal/process.h"
#include "hal/vfs.h"
#include "syscall_frame.h"
#include "sys_execve.h"
#include "kmalloc.h"
#include "paging.h"
#include "errno.h"
#include "fcntl.h"
#include <boot/bootparams.h>
#include <string.h>
#include "arch/x86_64/gdt.h"

extern BootParams g_bootParams;
extern Process *current_process;
extern Process *proc_alloc(void);
extern void proc_reap(Process *);
extern void proc_start(Process *, const struct syscall_frame *);
extern uint8_t *proc_fpu(Process *);
extern int  exec_elf_load(Process *, int, BootParams *, size_t, char **, char **, uint64_t *);
extern int  clone_user_address_space(uint64_t *, uint64_t *);

long sys_fork(struct syscall_frame *f)
{
    Process *par = current_process;
    Process *ch  = proc_alloc();
    if (!ch) return -ENOMEM;

    page_dir_t pd = create_user_pd();
    if (!pd.pd_virt) { proc_reap(ch); return -ENOMEM; }
    ch->page_directory = pd.pd_virt;
    ch->cr3            = pd.pd_phys;

    if (clone_user_address_space(par->page_directory, ch->page_directory) != 0) {
        proc_reap(ch); return -ENOMEM;
    }

    ch->mmap_base     = par->mmap_base;
    ch->brk_start     = par->brk_start;   ch->brk_end   = par->brk_end;
    ch->brk_cur       = par->brk_cur;     ch->brk_end_limit = par->brk_end_limit;
    ch->fs_base       = par->fs_base;     ch->gs_base   = par->gs_base;

    __asm__ volatile("fxsave (%0)" :: "r"(proc_fpu(ch)) : "memory");  /* parent's live FPU state */

    for (int i = 0; i < PROC_MAX_FDS; i++) {
        struct file *fl = par->fds[i];
        if (fl && fl != (struct file *)1) { ch->fds[i] = fl; fl->refcount++; }
    }
    ch->parent = par;
    proc_start(ch, f);          /* child resumes after the syscall with rax = 0 */
    return ch->pid;
}

/* ---- helpers shared with spawn ---- */
static int copy_user_str(char *dst, uint64_t u, size_t max)
{
    for (size_t i = 0; i < max - 1; i++) {
        uint8_t c;
        if (!copy_from_user_byte(&c, (const uint8_t *)u + i)) return -1;
        dst[i] = (char)c;
        if (!c) return 0;
    }
    dst[max - 1] = 0; return 0;
}

static int copy_user_strv(uint64_t uvec, char **out, int max)
{
    int n = 0;
    if (uvec) while (n < max) {
        char *u = NULL;
        if (!copy_from_user_ptr(&u, (char **)uvec + n) || !u) break;
        char *k = kmalloc(MAX_EXEC_ARG_LEN);
        if (!k) break;
        size_t i = 0;
        for (; i < MAX_EXEC_ARG_LEN - 1; i++) {
            uint8_t c;
            if (!copy_from_user_byte(&c, (uint8_t *)u + i)) break;
            k[i] = (char)c;
            if (!c) break;
        }
        k[i] = 0;
        out[n++] = k;
    }
    out[n] = NULL;
    return n;
}

static void free_strv(char **v, int n) { for (int i = 0; i < n; i++) kfree(v[i]); }

/* spawn(path, argv, envp, flags) -> child pid. Custom syscall 0x1001. */
long sys_spawn(uint64_t path_u, uint64_t argv_u, uint64_t envp_u, uint64_t flags)
{
    char  kpath[256];
    char *kargv[MAX_EXEC_ARGS + 1], *kenvp[MAX_EXEC_ARGS + 1];
    if (copy_user_str(kpath, path_u, sizeof(kpath)) < 0) return -EFAULT;
    int argc = copy_user_strv(argv_u, kargv, MAX_EXEC_ARGS);
    int envc = copy_user_strv(envp_u, kenvp, MAX_EXEC_ARGS);
    long ret;

    int fd = VFS_Open(kpath, O_RDONLY);          /* in the parent's fd table */
    if (fd < 0) { ret = fd; goto out; }

    Process *ch = proc_alloc();
    if (!ch) { VFS_Close(fd); ret = -ENOMEM; goto out; }
    ch->parent    = current_process;
    ch->auto_reap = (flags & SPAWN_DETACH) != 0;

    uint64_t old = 0;
    if (exec_elf_load(ch, fd, &g_bootParams, argc, kargv, kenvp, &old) != 0) {
        VFS_Close(fd); proc_reap(ch); ret = -ENOEXEC; goto out;
    }
    VFS_Close(fd);

    struct syscall_frame f;
    memset(&f, 0, sizeof(f));
    f.rip = ch->regs.rip;  f.rsp = ch->regs.rsp;
    f.rflags = 0x202;      f.cs = USER_CODE_SELECTOR;  f.ss = USER_DATA_SELECTOR;
    proc_start(ch, &f);
    ret = ch->pid;
out:
    free_strv(kargv, argc); free_strv(kenvp, envc);
    return ret;
}