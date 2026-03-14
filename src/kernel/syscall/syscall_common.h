#pragma once
#include <stdint.h>
#include <types.h>
#include <paging.h>

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

