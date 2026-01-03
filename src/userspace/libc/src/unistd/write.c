#include <unistd.h>
#include "syscall.h"

ssize_t write(int fd, const void *buf, size_t len)
{
    return (ssize_t)syscall(SYS_write,
                            (long)fd,
                            (long)buf,
                            (long)len);
}


// ssize_t write(int fd, const void *buf, size_t len)
// {
//     return syscall_write(fd, buf, len);
// }