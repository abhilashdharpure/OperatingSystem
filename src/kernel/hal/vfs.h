#pragma once
#include <stdint.h>
#include <stddef.h>
#include "file.h"
#include "string.h"

typedef int fd_t;
typedef long off_t;

/* Standard file descriptors */
#define VFS_FD_STDIN    0
#define VFS_FD_STDOUT   1
#define VFS_FD_STDERR   2
#define VFS_FD_DEBUG    3
#define VFS_FD_USER_BASE 4

/* Open flags (minimal set) */
#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR   0x3

#define MAX_VFS_ENTRIES 64
#define MAX_OPEN_FILES  32

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


/* Prototypes */
void VFS_Init(void);
int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data);
struct file *VFS_AllocFile(void);
int VFS_Open(const char *path, int flags);
int VFS_Read(fd_t fd, void *buf, size_t size);
int VFS_Write(fd_t file, uint8_t* data, size_t size);
int VFS_Close(fd_t fd);

int VFS_List(const char *path, void (*callback)(const dirent_t *));
int VFS_IsValidFd(int fd);
int VFS_CanRead(int fd);
int VFS_CanWrite(int fd);

struct file *VFS_GetFile(fd_t fd);
off_t VFS_Lseek(fd_t fd, off_t offset, int whence);
int VFS_AllocFd();
int VFS_Dup(fd_t oldfd);
int VFS_Dup2(fd_t oldfd, fd_t newfd);

int VFS_CreatePipe(fd_t fds[2]);
void VFS_SetFd(int fd, struct file *file);
