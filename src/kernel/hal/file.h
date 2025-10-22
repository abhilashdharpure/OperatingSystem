#pragma once

#include <stddef.h>
#include <stdint.h>

// typedef int fd_t;

struct file {
    const char *path;
    struct file_operations *fops;
    void *private_data;   // points to device or fs-specific struct
    size_t position;
};

struct file_operations {
    int  (*open)(struct file *file);
    int  (*close)(struct file *file);
    int  (*read)(struct file *file, void *buf, size_t size);
    int  (*write)(struct file *file, const void *buf, size_t size);
};