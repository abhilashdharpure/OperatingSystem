#include <stdint.h>
#include "errno.h"

uint64_t sys_memfd_create(uint64_t name_ptr, uint64_t flags)
{
    (void)name_ptr;
    (void)flags;
    // For now: not implemented
    return (uint64_t)-ENOSYS;
}
