#pragma once

#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "debug.h"

#ifndef EFAULT
#define EFAULT       14
#define ENAMETOOLONG 36
#define ENOSYS       38
#endif

#define K_PATH 256
#define AT_FDCWD     (-100)
#define AT_REMOVEDIR 0x200

/* copy a user path into a kernel buffer, make it absolute, strip trailing '/' */
static int copy_user_path(const char *u, char *k);

long sys_mkdir(const char *p, uint32_t mode);
long sys_rmdir(const char *p);
long sys_unlink(const char *p);
long sys_rename(const char *a, const char *b);

/* *at variants: only AT_FDCWD is supported */
long sys_mkdirat(int dfd, const char *p, uint32_t mode);
long sys_unlinkat(int dfd, const char *p, int flags);
long sys_renameat(int od, const char *a, int nd, const char *b);
long sys_renameat2(int od, const char *a, int nd, const char *b, unsigned flags);