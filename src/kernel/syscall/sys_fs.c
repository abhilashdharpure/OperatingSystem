#include "sys_fs.h"
#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "errno.h"


int VFS_Mkdir(const char *);  int VFS_Rmdir(const char *);
int VFS_Unlink(const char *); int VFS_Rename(const char *, const char *);
int unix_unbind_path(const char *path);

/* copy a user path into a kernel buffer, make it absolute, strip trailing '/' */
static int copy_user_path(const char *u, char *k)
{
    if (!u || (uintptr_t)u < 0x10000 || (uintptr_t)u >= 0x00007FFFFFFFFFFFULL)
    {
        return -EFAULT;
    }

    char tmp[K_PATH];
    if (!u || (uintptr_t)u >= 0x00007FFFFFFFFFFFULL) return -EFAULT;

    size_t i = 0;
    for (; i < K_PATH; i++) { tmp[i] = u[i]; if (!tmp[i]) break; }
    if (i == K_PATH) return -ENAMETOOLONG;
    if (i == 0) return -ENOENT;

    size_t o = 0;
    if (tmp[0] != '/') k[o++] = '/';                    // no per-process cwd yet: relative = from the root
    if (o + i >= K_PATH) return -ENAMETOOLONG;
    memcpy(k + o, tmp, i + 1);
    o += i;
    while (o > 1 && k[o - 1] == '/') k[--o] = 0;        // "dir/" -> "dir"
    return 0;
}

long sys_mkdir(const char *p, uint32_t mode)
{
    (void)mode;
    char k[K_PATH]; int r = copy_user_path(p, k);
    return r ? r : VFS_Mkdir(k);
}

long sys_rmdir(const char *p)
{
    char k[K_PATH]; int r = copy_user_path(p, k);
    return r ? r : VFS_Rmdir(k);
}

long sys_unlink(const char *p)
{
    char k[K_PATH];
    int r = copy_user_path(p, k);
    if (r) return r;
    if (unix_unbind_path(k) == 0) return 0;           // a socket name
    return VFS_Unlink(k);                             // a file on a mounted filesystem
}
long sys_rename(const char *a, const char *b)
{
    char ka[K_PATH], kb[K_PATH];
    int r = copy_user_path(a, ka); if (r) return r;
    r = copy_user_path(b, kb);     if (r) return r;
    return VFS_Rename(ka, kb);
}

/* *at variants: only AT_FDCWD is supported */
long sys_mkdirat(int dfd, const char *p, uint32_t mode) 
{
    return dfd == AT_FDCWD ? sys_mkdir(p, mode) : -ENOSYS;
}

long sys_unlinkat(int dfd, const char *p, int flags)
{
    if (dfd != AT_FDCWD) return -ENOSYS;
    return (flags & AT_REMOVEDIR) ? sys_rmdir(p) : sys_unlink(p);
}

long sys_renameat(int od, const char *a, int nd, const char *b)
{
    return (od == AT_FDCWD && nd == AT_FDCWD) ? sys_rename(a, b) : -ENOSYS;
}

long sys_renameat2(int od, const char *a, int nd, const char *b, unsigned flags)
{
    return flags ? -EINVAL : sys_renameat(od, a, nd, b);
}