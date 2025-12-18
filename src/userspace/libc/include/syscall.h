#pragma once
#include <stdint.h>

enum {
    SYS_write = 1,
    SYS_exit  = 2,
};

int syscall_write(int fd, const void *buf, uint32_t len);
void syscall_exit(int code);
