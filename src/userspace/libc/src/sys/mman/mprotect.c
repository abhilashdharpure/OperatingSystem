// libc/src/sys/mman/mprotect.c
#include <sys/mman.h>
#include <syscall.h>

int mprotect(void *addr, size_t len, int prot)
{
    long ret = syscall(SYS_mprotect,
                       (long)addr,
                       (long)len,
                       (long)prot);
    return (int)ret; // 0 on success, -1 on error
}
