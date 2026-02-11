#pragma once
#include <stddef.h>
#include <stdint.h>

struct iovec_k {
    void  *iov_base;
    size_t iov_len;
};

struct msghdr_k {
    void         *msg_name;
    size_t        msg_namelen;
    struct iovec_k *msg_iov;
    size_t        msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};
