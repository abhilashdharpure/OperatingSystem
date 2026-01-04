// libc/src/sys/mman/mmap.c
#include <sys/mman.h>
#include <syscall.h>

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, size_t offset)
{
    (void)addr;
    (void)fd;
    (void)offset;

    long ret = syscall(SYS_mmap,
                       (long)length,
                       (long)prot,
                       (long)flags,
                        0);
    if (ret == -1)
        return (void *)-1; // MAP_FAILED
    return (void *)ret;
}
