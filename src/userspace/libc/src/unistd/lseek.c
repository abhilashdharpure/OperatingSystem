#include <unistd.h>
#include <syscall.h>

off_t lseek(int fd, off_t offset, int whence)
{
    long r = syscall(SYS_lseek, fd, offset, whence);
    return (off_t)r;
}
