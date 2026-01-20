// libc/include/sys/mman.h
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "types.h"

#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

#define MAP_FAILED ((void *)-1)


void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t len, int prot);
int memfd_create(const char *name, unsigned int flags);
