#include <stdint.h>
#include <syscall.h>

int syscall_write(int fd, const void *buf, uint32_t len) {
    int ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_write), "b"(fd), "c"(buf), "d"(len)
        : "memory"
    );

    return ret;
}

void syscall_exit(int code) {
    asm volatile(
        "int $0x80"
        :
        : "a"(SYS_exit), "b"(code)
        : "memory"
    );

    while (1) { }
}
