// libc/src/sys/mman/mmap.c
#include <sys/mman.h>
#include <syscall.h>
#include <stddef.h>
#include <stdint.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return (void *)syscall6(SYS_mmap,
                            (long)addr,
                            (long)length,
                            (long)prot,
                            (long)flags,
                            (long)fd,
                            (long)offset);
}
