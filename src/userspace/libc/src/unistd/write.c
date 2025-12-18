#include <unistd.h>
#include <syscall.h>

ssize_t write(int fd, const void *buf, size_t len) {
    return (ssize_t)syscall_write(fd, buf, (uint32_t)len);
}
