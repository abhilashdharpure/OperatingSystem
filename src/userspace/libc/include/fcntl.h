#pragma once
#include <types.h>

#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_CREAT   0x0100
#define O_TRUNC   0x0200
#define O_APPEND  0x0400
#define O_NONBLOCK  0x0400

#define F_GETFL     3
#define F_SETFL     4

int fcntl(int fd, int cmd, long arg);
