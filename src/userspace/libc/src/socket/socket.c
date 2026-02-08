#include "sys/socket.h"
#include "sys/syscall.h"

int socket(int domain, int type, int protocol)
{
    return (int)syscall6(SYS_socket,
                         domain,
                         type,
                         protocol,
                         0, 0, 0);
}

int bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    return (int)syscall6(SYS_bind,
                         fd,
                         (long)addr,
                         len,
                         0, 0, 0);
}

int listen(int fd, int backlog)
{
    return (int)syscall6(SYS_listen,
                         fd,
                         backlog,
                         0, 0, 0, 0);
}

int accept(int fd, struct sockaddr *addr, socklen_t *len)
{
    return (int)syscall6(SYS_accept,
                         fd,
                         (long)addr,
                         (long)len,
                         0, 0, 0);
}

int connect(int fd, const struct sockaddr *addr, socklen_t len)
{
    return (int)syscall6(SYS_connect,
                         fd,
                         (long)addr,
                         len,
                         0, 0, 0);
}

ssize_t write(int fd, const void *buf, size_t len)
{
    return (ssize_t)syscall6(SYS_write,
                             fd,
                             (long)buf,
                             len,
                             0, 0, 0);
}

ssize_t read(int fd, void *buf, size_t len)
{
    return (ssize_t)syscall6(SYS_read,
                             fd,
                             (long)buf,
                             len,
                             0, 0, 0);
}

int close(int fd)
{
    return (int)syscall6(SYS_close,
                         fd,
                         0, 0, 0, 0, 0);
}
