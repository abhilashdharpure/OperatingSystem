#pragma once
#include <stdint.h>
#include <types.h>

#define PAGE_SIZE 0x1000
#define TICKS_PER_SEC 1000             // e.g. 1ms tick

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

#define MSR_FS_BASE 0xC0000100

// linux values
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003

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

