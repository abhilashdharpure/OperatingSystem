#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <debug.h>

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;
    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
            VGA_putc(data[i]);
        return size;

    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
            e9_putc(data[i]);
        return size;

    default:
        return -1;
    }
}

static int vfs_count = 0;
static struct file open_files[MAX_OPEN_FILES];

int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data)
{
    if (vfs_count >= MAX_VFS_ENTRIES)
    {
        log_error("VFS", "Max device entries reached!");
        return -1;
    }

    vfs_table[vfs_count].path = kstrdup(path); // path;
    vfs_table[vfs_count].fops = fops;
    vfs_table[vfs_count].private_data = private_data;

    log_info("VFS", "Registered device: %s (index=%d)", vfs_table[vfs_count].path, vfs_count);

    vfs_count++;


    return 0;
}

int VFS_Open(const char *path, int flags)
{
    log_info("VFS", "Trying to open: %s", path);

    for (int i = 0; i < vfs_count; i++) {
        log_info("VFS", "Checking %s (index=%d)", vfs_table[i].path, i);
        if (strcmp(vfs_table[i].path, path) == 0) {
            for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
                if (open_files[fd].path == NULL) {
                    open_files[fd].path = vfs_table[i].path;
                    open_files[fd].fops = vfs_table[i].fops;
                    open_files[fd].private_data = vfs_table[i].private_data;
                    open_files[fd].position = 0;

                    if (open_files[fd].fops->open)
                        open_files[fd].fops->open(&open_files[fd]);

                    return fd;
                }
            }
        }
    }
    return -1; // not found
}

int VFS_Read(fd_t fd, void *buf, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
        return -1;

    struct file *file = &open_files[fd];
    if (file->fops && file->fops->read)
        return file->fops->read(file, buf, size);

    return -1;
}

// int VFS_Write(fd_t fd, const void *buf, size_t size) {
//     if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
//         return -1;

//     struct file *file = &open_files[fd];
//     if (file->fops && file->fops->write)
//         return file->fops->write(file, buf, size);

//     return -1;
// }