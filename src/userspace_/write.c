#include <stdint.h>
#include "syscall/syscall.h"

int syscall(int num, int a, const void *b, uint32_t c);

int write(int fd, const void *buf, uint32_t len)
{
    return syscall(SYS_WRITE, fd, buf, len);
}
