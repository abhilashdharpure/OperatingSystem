#include <unistd.h>
#include <syscall.h>

int brk(void *addr)
{
    long ret = syscall(SYS_brk, (long)addr, 0, 0);
    // Linux returns new brk address on success, or old on failure.
    // Classical brk() returns 0 on success, -1 on failure.
    // We'll emulate that: success if ret == (long)addr.
    if (ret == (long)addr)
        return 0;
    return -1;
}

void *sbrk(intptr_t increment)
{
    long cur = syscall(SYS_brk, 0, 0, 0);
    if (cur == 0)
        return (void *)-1;

    long new = cur + increment;
    long ret = syscall(SYS_brk, new, 0, 0);

    if (ret != new)
        return (void *)-1;

    return (void *)cur;
}
