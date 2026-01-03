// libc/include/sys/mman.h
#pragma once
#include <stddef.h>
#include <stdint.h>

#define PROT_READ   0x1
#define PROT_WRITE  0x2

#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, size_t offset);
