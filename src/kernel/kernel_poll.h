#pragma once
#include <stdint.h>

struct pollfd {
    int   fd;
    short events;
    short revents;
};

/* Events */
#define POLLIN   0x0001
#define POLLOUT  0x0004
#define POLLERR  0x0008
#define POLLHUP  0x0010
#define POLLNVAL 0x0020
