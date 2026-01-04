#pragma once

#include <stddef.h>
#include <stdint.h>
#include <kstat.h>

#define NAME_MAX 100

typedef struct {
    char name[NAME_MAX];
    uint32_t inode;
    uint8_t type; // 0=file, 1=dir, 2=device, etc.
} dirent_t;

struct file {
    const char *path;           // full original path (e.g., "/bin/init")
    const char *subpath;        // path relative to mount (e.g., "bin/init")
    struct file_operations *fops;
    void *private_data;         // filesystem or device context
    size_t position;
    int refcount;               // number of FDs sharing this file
    int flags; 
};

struct file_operations {
    int  (*open)(struct file *file, int flags);
    int  (*close)(struct file *file);
    int  (*read)(struct file *file, void *buf, size_t size);
    int  (*write)(struct file *file, const void *buf, size_t size);
    int  (*ioctl)(struct file* f, unsigned long request, void* arg);
    int  (*readdir)(struct file *dir, dirent_t *);
    int  (*stat)(struct file *f, struct kstat *st);
    // int  (*readdir)(struct file *f, dirent_t *ent);
};
