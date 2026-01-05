#pragma once
#include <stdint.h>

#define SOCKETPAIR_BUF_SIZE 4096
#define AF_UNIX 1
#define SOCK_STREAM 1

#define SP_MAX_FDS 16

typedef struct socketpair {
    char buf0[SOCKETPAIR_BUF_SIZE];
    char buf1[SOCKETPAIR_BUF_SIZE];
    uint32_t head0, tail0;
    uint32_t head1, tail1;

    int fdq0[SP_MAX_FDS];
    int fdq0_head, fdq0_tail;

    int fdq1[SP_MAX_FDS];
    int fdq1_head, fdq1_tail;

    int refcount;
} socketpair_t;

extern struct file_operations socketpair_fops;