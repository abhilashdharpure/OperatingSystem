#include <sys/syscall.h>
#include <fcntl.h>   // define O_RDONLY, O_WRONLY, etc.
#include <stdint.h>

int open(const char *path, int flags, int mode)
{
    return (int)syscall(SYS_open,
                        (long)path,
                        (long)flags,
                        (long)mode,
                        0);
}
