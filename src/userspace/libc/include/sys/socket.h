#pragma once

#include <stddef.h>
#include <stdint.h>
#include <types.h>
#include <sys/un.h>   // for sockaddr_un, socklen_t etc.

// Address families
#define AF_UNIX   1
#define AF_LOCAL  AF_UNIX

// Socket types
#define SOCK_STREAM 1
#define SOCK_DGRAM  2

// Socket levels
#define SOL_SOCKET  1

// Control message types
#define SCM_RIGHTS  1


#include <sys/uio.h>   // for struct iovec
#include <types.h>

struct msghdr {
    void         *msg_name;
    socklen_t     msg_namelen;
    struct iovec *msg_iov;
    size_t        msg_iovlen;
    void         *msg_control;
    size_t        msg_controllen;
    int           msg_flags;
};

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int fd, struct msghdr *msg, int flags);

struct cmsghdr {
    size_t cmsg_len;
    int    cmsg_level;
    int    cmsg_type;
};

// CMSG helpers
#define CMSG_ALIGN(len)   (((len) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1))
#define CMSG_SPACE(len)   (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_LEN(len)     (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))

#define CMSG_FIRSTHDR(mhdr) \
    ((mhdr)->msg_controllen >= sizeof(struct cmsghdr) ? \
        (struct cmsghdr *)(mhdr)->msg_control : \
        (struct cmsghdr *)0)

#define CMSG_DATA(cmsg) \
    ((unsigned char *)(cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr)))

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr {
    uint16_t sa_family;
    char     sa_data[14];
};

int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *addr, socklen_t len);
int listen(int fd, int backlog);
int accept(int fd, struct sockaddr *addr, socklen_t *len);
int connect(int fd, const struct sockaddr *addr, socklen_t len);

int socketpair(int domain, int type, int protocol, int sv[2]);
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int fd, struct msghdr *msg, int flags);

#ifdef __cplusplus
}
#endif
