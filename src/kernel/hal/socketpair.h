#pragma once
#include <stdint.h>

#define SOCKETPAIR_BUF_SIZE 4096
#define AF_UNIX 1
#define SOCK_STREAM 1

typedef struct socketpair {
    uint8_t buf0[SOCKETPAIR_BUF_SIZE];
    uint8_t buf1[SOCKETPAIR_BUF_SIZE];

    uint32_t head0, tail0; // incoming for fd0
    uint32_t head1, tail1; // incoming for fd1
} socketpair_t;

extern struct file_operations socketpair_fops;