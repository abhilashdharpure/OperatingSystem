#include <unistd.h>
#include <syscall.h>

int pipe(int fds[2])
{
    long r = syscall(SYS_pipe, (long)fds, 0, 0, 0);
    return (int)r;
}
