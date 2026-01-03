#pragma once
#include <stdint.h>

enum {
    SYS_test  = 0,
    SYS_write = 1,
    SYS_exit  = 2,
};

int syscall_write(int fd, const void *buf, uint32_t len);
void syscall_exit(int code);

long syscall0(long n);
long syscall3(long n, long a, long b, long c);
long syscall(long n, long a, long b, long c);

