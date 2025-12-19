#pragma once
#include <stdint.h>
#include <stddef.h>
#include "file.h"
#include "string.h"

typedef int fd_t;

/* Standard file descriptors */
#define VFS_FD_STDIN    0
#define VFS_FD_STDOUT   1
#define VFS_FD_STDERR   2
#define VFS_FD_DEBUG    3

/* Open flags (minimal set) */
#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR   0x3

int VFS_Write(fd_t file, uint8_t* data, size_t size);

#define MAX_VFS_ENTRIES 64
#define MAX_OPEN_FILES  32

/* Prototypes */
int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data);
int VFS_Open(const char *path, int flags);
int VFS_Read(fd_t fd, void *buf, size_t size);
int VFS_Close(fd_t fd);

int VFS_List(const char *path, void (*callback)(const dirent_t *));