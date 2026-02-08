#include <sys/syscall.h>
#include <unistd.h>

int close(int fd)
{
    return (int)syscall6(SYS_close, fd, 0, 0, 0, 0, 0);
}
