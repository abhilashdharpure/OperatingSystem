// libc/src/sys/poll/poll.c
#include <sys/poll.h>
#include <syscall.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    long ret = syscall(SYS_poll,
                       (long)fds,
                       (long)nfds,
                       (long)timeout);
    return (int)ret;
}
