#include <syscall.h>
#include <unistd.h>
#include <stdint.h>

ssize_t read(int fd, void *buf, size_t len)
{
    return (ssize_t)syscall(SYS_read,
                            (long)fd,
                            (long)buf,
                            (long)len,
                            0);
}
