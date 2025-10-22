#pragma once
#include <stdint.h>
#include <stddef.h>
#include "file.h"
#include "util/string.h"

typedef int fd_t;

#define VFS_FD_STDIN    0
#define VFS_FD_STDOUT   1
#define VFS_FD_STDERR   2
#define VFS_FD_DEBUG    3

int VFS_Write(fd_t file, uint8_t* data, size_t size);



#define MAX_VFS_ENTRIES 64
#define MAX_OPEN_FILES 32

static struct {
    const char *path;
    struct file_operations *fops;
    void *private_data;
} vfs_table[MAX_VFS_ENTRIES];


int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data);
int VFS_Read(fd_t fd, void *buf, size_t size);
// int VFS_Write(fd_t fd, const void *buf, size_t size);