#pragma once
#include <stdint.h>
#include "hal/vfs.h"

typedef union epoll_data {
    void     *ptr;
    int       fd;
    uint32_t  u32;
    uint64_t  u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;
    epoll_data_t data;
};

/* epoll_ctl operations */
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

/* epoll events (minimal set) */
#define EPOLLIN   0x001
#define EPOLLOUT  0x004
// you can add more later if needed

struct epoll_watch {
    int              fd;
    uint32_t         events;
    epoll_data_t     data;
};

struct epoll_instance {
    int                nfds;
    struct epoll_watch *watches;
};

extern struct file_operations epoll_fops;

long sys_epoll_create1(int flags);
long sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event *user_ev);
long sys_epoll_wait(int epfd, struct epoll_event *user_events,
                    int maxevents, int timeout);
