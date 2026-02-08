#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

ssize_t read(int fd, void *buf, size_t len)
{
    return (ssize_t)syscall6(SYS_read, fd, (long)buf, len, 0, 0, 0);
}
