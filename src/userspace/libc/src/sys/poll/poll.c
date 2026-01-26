// libc/src/sys/poll/poll.c
#include <sys/poll.h>
#include <sys/syscall.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    long ret = syscall(SYS_poll,
                       (long)fds,
                       (long)nfds,
                       (long)timeout,
                        0);
    return (int)ret;
}
