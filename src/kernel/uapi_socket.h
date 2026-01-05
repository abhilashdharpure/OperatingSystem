#pragma once

#include <stddef.h>
#include <stdint.h>

// Must match libc/include/sys/socket.h

typedef unsigned int socklen_t;

struct iovec_k {
    void  *iov_base;
    size_t iov_len;
};

struct msghdr_k {
    void         *msg_name;
    socklen_t     msg_namelen;
    struct iovec_k *msg_iov;
    size_t        msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};

struct cmsghdr_k {
    size_t cmsg_len;
    int    cmsg_level;
    int    cmsg_type;
};

#define SOL_SOCKET  1
#define SCM_RIGHTS  1

#define CMSG_ALIGN_K(len)   (((len) + sizeof(size_t)-1) & ~(sizeof(size_t)-1))
#define CMSG_SPACE_K(len)   (CMSG_ALIGN_K(sizeof(struct cmsghdr_k)) + CMSG_ALIGN_K(len))
#define CMSG_LEN_K(len)     (CMSG_ALIGN_K(sizeof(struct cmsghdr_k)) + (len))
