#include <syscall.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>


ssize_t write(int fd, const void *buf, size_t len) {
    return syscall3(SYS_write, fd, (long)buf, len);
}

ssize_t read(int fd, void *buf, size_t len) {
    return syscall3(SYS_read, fd, (long)buf, len);
}

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags) {
    return syscall3(SYS_sendmsg, fd, (long)msg, flags);
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags) {
    return syscall3(SYS_recvmsg, fd, (long)msg, flags);
}

int close(int fd) {
    return syscall1(SYS_close, fd);
}

int socket(int domain, int type, int protocol) {
    return syscall3(SYS_socket, domain, type, protocol);
}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
    return syscall3(SYS_bind, fd, (long)addr, len);
}

int listen(int fd, int backlog) {
    return syscall2(SYS_listen, fd, backlog);
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
    return syscall3(SYS_accept, fd, (long)addr, (long)len);
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
    return syscall3(SYS_connect, fd, (long)addr, len);
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return syscall3(SYS_poll, (long)fds, nfds, timeout);
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
    return syscall4(SYS_socketpair, domain, type, protocol, (long)sv);
}

int ftruncate(int fd, off_t len) {
    return syscall2(SYS_ftruncate, fd, len);
}

off_t lseek(int fd, off_t off, int whence) {
    return syscall3(SYS_lseek, fd, off, whence);
}
