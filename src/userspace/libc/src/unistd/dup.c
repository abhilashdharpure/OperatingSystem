#include <unistd.h>
#include <sys/syscall.h>

int dup(int oldfd)
{
    long r = syscall(SYS_dup, oldfd, 0, 0, 0);
    return (int)r;
}

int dup2(int oldfd, int newfd)
{
    long r = syscall(SYS_dup2, oldfd, newfd, 0, 0);
    return (int)r;
}
