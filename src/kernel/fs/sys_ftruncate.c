#include <stdint.h>
#include <stddef.h>
#include "memfd.h"
#include "hal/vfs.h"
#include "kmalloc.h"
#include "errno.h"
#include "debug.h"
#include "memfd.h"

static int memfd_truncate(struct file *f, off_t length)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m || length < 0)
        return -EINVAL;

    size_t new_size = (size_t)length;

    if (new_size > m->capacity) {
        int r = memfd_ensure_capacity(m, new_size);
        if (r < 0)
            return r;
    }

    m->size = new_size;
    if (f->position > (off_t)m->size)
        f->position = (off_t)m->size;

    return 0;
}

uint64_t sys_ftruncate(uint64_t fd_arg, uint64_t length_arg)
{
    int fd = (int)fd_arg;
    off_t length = (off_t)length_arg;

    struct file *f = VFS_GetFile(fd);
    if (!f)
        return (uint64_t)-EBADF;

    if (!f->fops || !f->fops->write)
        return (uint64_t)-EINVAL;

    // memfd implements truncate via private function
    if (f->fops->stat == memfd_stat) {
        // memfd path
        extern int memfd_truncate(struct file *f, off_t length);
        return (uint64_t)memfd_truncate(f, length);
    }

    // fallback: regular files do not support truncate yet
    return (uint64_t)-EINVAL;
}
