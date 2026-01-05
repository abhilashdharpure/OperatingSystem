#pragma once

#include <stddef.h>
#include <types.h>

#define AF_UNIX 1
#define SOCK_STREAM 1

// Socket levels
#define SOL_SOCKET  1

// Control message types
#define SCM_RIGHTS  1

// For msghdr/cmsghdr
#define CMSG_ALIGN(len)   (((len) + sizeof(size_t)-1) & ~(sizeof(size_t)-1))
#define CMSG_SPACE(len)   (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_LEN(len)     (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

struct msghdr {
    void         *msg_name;
    socklen_t     msg_namelen;
    struct iovec *msg_iov;
    size_t        msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};

struct cmsghdr {
    size_t cmsg_len;
    int    cmsg_level;
    int    cmsg_type;
    // unsigned char cmsg_data[]; // flexible array
};

int socketpair(int domain, int type, int protocol, int sv[2]);
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int fd, struct msghdr *msg, int flags);
