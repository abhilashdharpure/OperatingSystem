#include <syscall.h>
#include <unistd.h>

int close(int fd)
{
    return (int)syscall(SYS_close,
                        (long)fd,
                        0,
                        0,
                        0);
}
