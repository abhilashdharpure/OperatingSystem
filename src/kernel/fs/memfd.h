#pragma once

#include <stddef.h>
#include <stdint.h>
#include <hal/file.h>   // for struct file, struct file_operations

typedef struct memfd {
    char   *data;      // backing buffer
    size_t  size;      // logical size (file size)
    size_t  capacity;  // allocated bytes in data
    int     refcount;
} memfd_t;

// Expose fops to VFS
extern struct file_operations memfd_fops;

// Syscall
uint64_t sys_memfd_create(uint64_t name_ptr, uint64_t flags);


static int memfd_close(struct file *f);
