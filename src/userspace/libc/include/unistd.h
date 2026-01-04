#pragma once

#include <stddef.h>
#include <stdint.h>
#include <types.h>

ssize_t write(int fd, const void *buf, size_t len);
void _exit(int code);
void exit(int code);
int     open(const char *path, int flags, int mode);
ssize_t read(int fd, void *buf, size_t len);
int     close(int fd);
int brk(void *addr);
void *sbrk(intptr_t increment);
off_t lseek(int fd, off_t offset, int whence);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int pipe(int fds[2]);
void klog(const char *msg);
