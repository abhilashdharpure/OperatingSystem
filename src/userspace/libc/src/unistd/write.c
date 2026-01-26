#include <unistd.h>
#include "sys/syscall.h"

ssize_t write(int fd, const void *buf, size_t len)
{
    return (ssize_t)syscall(SYS_write,
                            (long)fd,
                            (long)buf,
                            (long)len,
                            0);
}