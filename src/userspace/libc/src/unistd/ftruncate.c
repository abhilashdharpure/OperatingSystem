#include <unistd.h>
#include <syscall.h>

int ftruncate(int fd, off_t length)
{
    return (int)syscall(SYS_ftruncate, fd, length, 0, 0);
}
