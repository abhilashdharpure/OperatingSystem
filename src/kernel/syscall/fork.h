#pragma once

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

#define SPAWN_DETACH 1

long sys_fork(struct syscall_frame *f);
/* ---- helpers shared with spawn ---- */
static int copy_user_str(char *dst, uint64_t u, size_t max);

static int copy_user_strv(uint64_t uvec, char **out, int max);
static void free_strv(char **v, int n);

/* spawn(path, argv, envp, flags) -> child pid. Custom syscall 0x1001. */
long sys_spawn(uint64_t path_u, uint64_t argv_u, uint64_t envp_u, uint64_t flags);