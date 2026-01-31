#include "utsname.h"
#include <string.h>
#include "debug.h"
#include "errno.h"
#include "hal/process.h"

#define RLIM_INFINITY  (~(uint64_t)0)


struct rlimit64 {
    uint64_t rlim_cur;
    uint64_t rlim_max;
};

int sys_getpid(void) {
    return 1;   // single‑process system for now
}

int sys_getppid(void) {
    return 0;
}

int sys_uname(struct utsname *u) {
    strcpy(u->sysname, "myos");
    strcpy(u->nodename, "myos");
    strcpy(u->release, "0.1");
    strcpy(u->version, "0.1");
    strcpy(u->machine, "x86_64");
    return 0;
}

int sys_getcwd(char *buf, size_t size) {
    if (size < 2) return -EINVAL;
    buf[0] = '/';
    buf[1] = '\0';
    return 1;
}

int sys_madvise(void *addr, size_t len, int advice) {
    return 0;   // musl is fine with this
}

int sys_set_tid_address(int *tidptr) {
    return 1;
}

// int sys_prlimit64(pid_t pid, int resource,
//                   const struct rlimit *new_limit,
//                   struct rlimit *old_limit) {
//     return 0;
// }

// int sys_getrandom(void *buf, size_t len, unsigned flags) {
//     // simple PRNG for now
//     for (size_t i = 0; i < len; i++)
//         ((unsigned char*)buf)[i] = (i * 37 + 13) & 0xFF;
//     return len;
// }

int sys_exit_group(int code) {
    // same as sys_exit for now
    sys_exit(code);
    return 0;
}

long sys_ioctl(int fd, unsigned long req, unsigned long arg) {
    return -ENOTTY; // or 0 if you want to lie
}

long sys_prlimit64(pid_t pid, int resource,
                   const struct rlimit64 *newlim,
                   struct rlimit64 *oldlim)
{
    if (oldlim) {
        oldlim->rlim_cur = RLIM_INFINITY;
        oldlim->rlim_max = RLIM_INFINITY;
    }
    return 0;
}

long sys_getrandom(void *buf, size_t len, unsigned int flags) {
    // temporary: deterministic junk
    uint8_t *p = buf;
    for (size_t i = 0; i < len; i++) p[i] = 0x42;
    return (long)len;
}

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

long sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
    long total = 0;

    for (int i = 0; i < iovcnt; i++) {
        // you’ll want a safe copy_from_user here eventually
        const struct iovec *u = &iov[i];
        if (!u->iov_base || u->iov_len == 0)
            continue;

        long n = sys_write(fd, u->iov_base, u->iov_len);
        if (n < 0)
            return (total > 0) ? total : n;

        total += n;
        if (n < (long)u->iov_len)
            break; // short write
    }

    return total;
}
