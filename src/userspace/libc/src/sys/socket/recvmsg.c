#include <sys/socket.h>
#include <types.h>
#include <syscall.h>

ssize_t recvmsg(int fd, struct msghdr *msg, int flags)
{
    return (ssize_t)syscall(SYS_recvmsg,
                            (long)fd,
                            (long)msg,
                            (long)flags,
                            0);
}
