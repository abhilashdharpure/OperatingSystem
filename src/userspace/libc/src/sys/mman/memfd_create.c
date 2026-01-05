#include "sys/mman.h"
#include "syscall.h"

int memfd_create(const char *name, unsigned int flags)
{
    return (int)syscall(SYS_memfd_create, (long)name, flags, 0, 0);
}
