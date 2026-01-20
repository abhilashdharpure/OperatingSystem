#pragma once

#include <stddef.h>
#include <stdint.h>
#include <hal/file.h>   // for struct file, struct file_operations

typedef struct memfd {
    uint64_t *pages;   // physical page frames
    size_t    npages;  // number of pages
    size_t    size;    // logical size in bytes
    int       refcount;
} memfd_t;

// Expose fops to VFS
extern struct file_operations memfd_fops;

// Syscall
uint64_t sys_memfd_create(uint64_t name_ptr, uint64_t flags);

int memfd_stat(struct file *f, struct kstat *st);
int memfd_ensure_capacity(memfd_t *m, size_t new_capacity);

static int memfd_close(struct file *f);
