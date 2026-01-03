#include "syscall_common.h"
#include "debug.h"
#include "hal/vfs.h"

ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len)
{
    (void)fd;
    for (uint64_t i = 0; i < len; ++i)
    {
        serial_putc(buf[i]);
    }

    return len;
}

__attribute__((noreturn))
void sys_exit(uint64_t code)
{
    log_info("SYSCALL", "userspace exited with code %llu",
             (unsigned long long)code);

    // Instead of halting, spin with a heartbeat
    for (;;) {
        log_info("SYSCALL", "spinning after exit...");
        for (volatile uint64_t i = 0; i < 100000000; ++i) { }
    }
}

int64_t sys_open(const char *path, uint64_t flags, uint64_t mode)
{
    (void)mode; // ignore for now

    log_info("SYSCALL", "sys_open path=%s flags=%llx",
             path, (unsigned long long)flags);

    int fd = VFS_Open(path, (int)flags);
    if (fd < 0)
        return -1;

    return fd;
}

ssize_t sys_read(uint64_t fd, void *buf, uint64_t len)
{
    log_info("SYSCALL", "sys_read fd=%llu len=%llu",
             (unsigned long long)fd, (unsigned long long)len);

    return VFS_Read((int)fd, buf, (size_t)len);
}

int64_t sys_close(uint64_t fd)
{
    log_info("SYSCALL", "sys_close fd=%llu",
             (unsigned long long)fd);

    return VFS_Close((int)fd);
}
