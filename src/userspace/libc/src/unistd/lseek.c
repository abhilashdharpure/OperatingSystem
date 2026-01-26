#include <unistd.h>
#include <sys/syscall.h>

off_t lseek(int fd, off_t offset, int whence)
{
    long r = syscall(SYS_lseek, fd, offset, whence, 0);
    return (off_t)r;
}
