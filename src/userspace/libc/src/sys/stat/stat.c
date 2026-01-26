#include <sys/stat.h>
#include <sys/syscall.h>

int stat(const char *path, struct stat *buf)
{
    long r = syscall(SYS_stat, (long)path, (long)buf, 0, 0);
    return (int)r;
}

int fstat(int fd, struct stat *buf)
{
    long r = syscall(SYS_fstat, fd, (long)buf, 0, 0);
    return (int)r;
}
