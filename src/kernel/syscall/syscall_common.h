#pragma once
#include <stdint.h>
#include <types.h>
#include <paging.h>
#include <string.h>
#include <errno.h>
#include <syscall/epoll.h>

struct sigaction {
    void (*sa_handler)(int);
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    unsigned long sa_mask[1]; // enough for now
};

typedef unsigned long sigset_t;


ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len);

__attribute__((noreturn))
void sys_exit(uint64_t code);

int64_t sys_open(const char *path, uint64_t flags, uint64_t mode);
ssize_t sys_read(uint64_t fd, void *buf, uint64_t len);
int64_t sys_close(uint64_t fd);
uint64_t sys_mmap(uint64_t addr,
                  uint64_t length,
                  uint64_t prot,
                  uint64_t flags,
                  uint64_t fd,
                  uint64_t offset);
uint64_t sys_mprotect(uint64_t addr, uint64_t length, uint64_t prot);

uint64_t sys_brk(uint64_t new_brk);
uint64_t sys_poll(uint64_t ufds_ptr,
                  uint64_t nfds,
                  uint64_t timeout_ms);

long sys_getpriority(int which, int who);
long sys_faccessat(int dirfd, const char *path, int mode, int flags);

long sys_getuid(void);
long sys_getgid(void);
long sys_geteuid(void);
long sys_getegid(void);
long sys_rt_sigaction(int signum,
                      const struct sigaction *act,
                      struct sigaction *oldact,
                      size_t sigsetsize);
long sys_rt_sigprocmask(int how,
                        const sigset_t *set,
                        sigset_t *oldset,
                        size_t sigsetsize);
long sys_tkill(int tid, int sig);
long sys_clone(unsigned long flags,
               void *child_stack,
               void *ptid,
               void *ctid,
               void *regs);
long sys_membarrier(int cmd, int flags);

long sys_pwrite(uint64_t fd,
                       const char *buf,
                       uint64_t len,
                       uint64_t pos_l,
                       uint64_t pos_h);
long sys_pwritev_compat(uint64_t fd,
                        uint64_t iov_user,
                        uint64_t vlen,
                        uint64_t pos_l,
                        uint64_t pos_h,
                        uint64_t unused);
long sys_pread(uint64_t fd,
               char *buf,
               uint64_t len,
               uint64_t offset,
               uint64_t unused);
long sys_preadv2(uint64_t fd,       
                 uint64_t iov_user,
                 uint64_t vlen,
                 uint64_t offset,
                 uint64_t flags,
                 uint64_t unused);

int eventfd2(unsigned int initval, int flags);

uint64_t mmap_fixed(uint64_t addr, uint64_t length, uint64_t prot);
uint64_t sys_mmap(uint64_t addr,
                  uint64_t length,
                  uint64_t prot,
                  uint64_t flags,
                  uint64_t fd,
                  uint64_t offset);
uint64_t sys_munmap(uint64_t addr, uint64_t length);
