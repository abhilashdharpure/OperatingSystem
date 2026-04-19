#include "utsname.h"
#include <string.h>
#include "debug.h"
#include "errno.h"
#include "fcntl.h"
#include "hal/process.h"

#include "syscall/termios.h"

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

long sys_set_tid_address(int *tidptr)
{
    (void)tidptr;  // unused for now
    return current_process->pid;
}


int sys_exit_group(int code) {
    // same as sys_exit for now
    sys_exit(code);
    return 0;
}

// long sys_ioctl(int fd, unsigned long req, unsigned long arg)
// {
//     log_info("IOCTL", "sys_ioctl called fd = %d, req = %ld, arg = %ld", fd, req, arg);
//     struct file *f = VFS_GetFile(fd);
//     if (!f || !f->fops || !f->fops->ioctl)
//     {
//         log_info("IOCTL", "sys_ioctl returning -ENOTTY");
//         return -ENOTTY;
//     }
        
//     long returnValue = f->fops->ioctl(f, (int)req, (void *)arg);
//     log_info("IOCTL", "sys_ioctl returning %d", returnValue);

//     // log_info("SYSCALL", "writev fd=%d iov=%p cnt=%d", fd, user_iov, iovcnt);


//     return returnValue;
// }

long sys_ioctl(uint64_t fd, uint64_t request, uint64_t arg)
{
    log_info("SYSCALL", "sys_ioctl fd=%llu req=0x%llx arg=0x%llx",
             (unsigned long long)fd,
             (unsigned long long)request,
             (unsigned long long)arg);

    // Validate FD
    if (!VFS_IsValidFd((int)fd)) {
        // Allow stdio silently (VERY IMPORTANT for libc)
        if (fd == VFS_FD_STDIN ||
            fd == VFS_FD_STDOUT ||
            fd == VFS_FD_STDERR ||
            fd == VFS_FD_DEBUG)
        {
            log_info("SYSCALL", "sys_ioctl: fake success for stdio fd=%llu", fd);
            return 0;
        }

        return -EBADF;
    }

    struct file *f = VFS_GetFile((int)fd);
    if (!f)
        return -EBADF;

    // ✅ 1. If driver provides ioctl → use it
    if (f->fops && f->fops->ioctl) {
        return f->fops->ioctl(f, request, arg);
    }

    // ✅ 2. Generic fallback handling (VERY IMPORTANT)
    switch (request)
    {
        // -----------------------------
        // Terminal / TTY (glibc expects this)
        // -----------------------------
        case 0x5401: // TCGETS
        case 0x5402: // TCSETS
        case 0x5403: // TCSETSW
        case 0x5404: // TCSETSF
        case 0x5405: // TCGETA
        case 0x5406: // TCSETA
        case 0x5407: // TCSETAW
        case 0x5408: // TCSETAF
        case 0x5413: // TIOCGWINSZ
        {
            log_info("SYSCALL", "sys_ioctl: fake TTY ioctl 0x%llx", request);

            // Optional: fake window size
            if (request == 0x5413 && arg) {
                struct winsize {
                    uint16_t ws_row;
                    uint16_t ws_col;
                    uint16_t ws_xpixel;
                    uint16_t ws_ypixel;
                };

                struct winsize ws = {
                    .ws_row = 25,
                    .ws_col = 80,
                    .ws_xpixel = 0,
                    .ws_ypixel = 0
                };

                copy_to_user(arg, &ws, sizeof(ws));
            }

            return 0;
        }

        // -----------------------------
        // FIONBIO (non-blocking mode)
        // -----------------------------
        case 0x5421: // FIONBIO
        {
            int val = 0;
            if (copy_from_user(&val, (void *)arg, sizeof(int)) != 0)
                return -EFAULT;

            if (val)
                f->flags |= O_NONBLOCK;
            else
                f->flags &= ~O_NONBLOCK;

            return 0;
        }

        // -----------------------------
        // FIONREAD (bytes available)
        // -----------------------------
        case 0x541B:
        {
            int available = 0;

            // If driver supports poll/read, you can improve this later
            copy_to_user(arg, &available, sizeof(int));
            return 0;
        }

        default:
            log_warning("SYSCALL", "sys_ioctl: unsupported req=0x%llx", request);
            return -ENOTTY;  // Linux standard
    }
}

// long sys_ioctl(int fd, unsigned long req, unsigned long arg)
// {
//     log_info("IOCTL", "sys_ioctl called fd = %d, req = %ld, arg = %ld", fd, req, arg);
//     if ((fd == 1 || fd == 2) && req == 0x5413) { // TCGETS on stdout/stderr
//         struct termios *t = (struct termios *)arg;
//         memset(t, 0, sizeof(*t));


//         // TODO: if you have copy_to_user, use that:
//         // if (copy_to_user((void *)arg, &kterm, sizeof(kterm)) != 0)
//         //     return -EFAULT;

//         log_info("SYSCALL", "sys_ioctl returning 0");

//         return 0;
//     }

//     struct file *f = VFS_GetFile(fd);
//     if (!f || !f->fops || !f->fops->ioctl)
//     {
//         log_info("IOCTL", "sys_ioctl returning -ENOTTY");
//         return -ENOTTY;
//     }

//     long returnValue = f->fops->ioctl(f, (int)req, (void *)arg);
//     log_info("SYSCALL", "sys_ioctl returnValue=%d", returnValue);
//     return returnValue;
// }


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

long sys_getrandom(void *buf, size_t len, unsigned int flags)
{
    // temporary: deterministic junk
    uint8_t *p = buf;
    for (size_t i = 0; i < len; i++) p[i] = 0x42;
    return (long)len;
}

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

long sys_writev(int fd, const struct iovec *user_iov, int iovcnt)
{
    log_info("SYSCALL", "writev fd=%d iov=%p cnt=%d", fd, user_iov, iovcnt);
    if (iovcnt <= 0 || iovcnt > 64)
        return -EINVAL;

    struct iovec kiov[64];

    // Copy iovec array from user
    for (int i = 0; i < iovcnt; i++) {
        if (copy_from_user(&kiov[i], &user_iov[i], sizeof(struct iovec)) != 0) {
            log_info("SYSCALL", "writev failed to copy iovec %d, returning -EFAULT", i);
            return -EFAULT;
        }

        log_info("SYSCALL", " iov[%d].base=%p len=%llu",
                 i,
                 kiov[i].iov_base,
                 (unsigned long long)kiov[i].iov_len);
    }

    long total = 0;

    for (int i = 0; i < iovcnt; i++) {
        if (!kiov[i].iov_base || kiov[i].iov_len == 0)
            continue;

        long n = sys_write(fd, kiov[i].iov_base, kiov[i].iov_len);
        if (n < 0)
            return (total > 0) ? total : n;

        total += n;
        if (n < (long)kiov[i].iov_len)
            break;
    }

    return total;
}
