#include <stdint.h>
#include "errno.h"

uint64_t sys_sendmsg(uint64_t fd, uint64_t msg_ptr, uint64_t flags)
{
    (void)fd;
    (void)msg_ptr;
    (void)flags;
    // For now: not implemented
    return (uint64_t)-ENOSYS;
}

uint64_t sys_recvmsg(uint64_t fd, uint64_t msg_ptr, uint64_t flags)
{
    (void)fd;
    (void)msg_ptr;
    (void)flags;
    // For now: not implemented
    return (uint64_t)-ENOSYS;
}
