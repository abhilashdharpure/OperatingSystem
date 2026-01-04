// libc/src/sys/mman/munmap.c
#include <sys/mman.h>
#include <syscall.h>

int munmap(void *addr, size_t length)
{
    long ret = syscall(SYS_munmap,
                       (long)addr,
                       (long)length,
                       0,0);
    return (int)ret;  // 0 on success, -1 on error
}
