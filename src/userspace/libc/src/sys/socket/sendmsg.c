#include <sys/socket.h>
#include <syscall.h>

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags)
{
    return (ssize_t)syscall(SYS_sendmsg, fd, (long)msg, flags, 0);
}
