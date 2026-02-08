#include <unistd.h>
#include "sys/syscall.h"

ssize_t write(int fd, const void *buf, size_t len)
{
    return (ssize_t)syscall6(SYS_write, fd, (long)buf, len, 0, 0, 0);
}