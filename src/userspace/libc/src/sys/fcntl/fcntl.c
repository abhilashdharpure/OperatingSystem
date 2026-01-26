#include <fcntl.h>
#include <sys/syscall.h>

int fcntl(int fd, int cmd, long arg)
{
    long r = syscall(SYS_fcntl, fd, cmd, arg, 0);
    return (int)r;
}
