#pragma once
#include <stdint.h>
#include <types.h>

enum {
    SYS_test   = 0,
    SYS_write  = 1,
    SYS_exit   = 2,
    SYS_open   = 3,
    SYS_read   = 4,
    SYS_close  = 5,
    SYS_mmap   = 6
    // later: SYS_mmap, SYS_poll, SYS_socket, ...
};


ssize_t syscall_write(int fd, const void *buf, size_t len);
// void syscall_exit(int code);

long syscall0(long n);
long syscall3(long n, long a, long b, long c);
long syscall(long n, long a, long b, long c);
