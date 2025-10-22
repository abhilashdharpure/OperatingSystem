#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>

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

int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data) {
    if (vfs_count >= MAX_VFS_ENTRIES)
        return -1;

    vfs_table[vfs_count].path = path;
    vfs_table[vfs_count].fops = fops;
    vfs_table[vfs_count].private_data = private_data;
    vfs_count++;

    return 0;
}




int VFS_Open(const char *path, int flags) {
    for (int i = 0; i < vfs_count; i++) {
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

int VFS_Read(fd_t fd, void *buf, size_t size) {
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