#include <stdint.h>
#include <stddef.h>
#include "memfd.h"
#include "hal/vfs.h"
#include "kmalloc.h"
#include "errno.h"
#include "debug.h"

// Helpers to grow the buffer

uint64_t sys_memfd_create(uint64_t name_ptr, uint64_t flags)
{
    (void)name_ptr;
    (void)flags;

    memfd_t *m = kmalloc(sizeof(memfd_t));
    if (!m)
        return (uint64_t)-ENOMEM;

    m->data = NULL;
    m->size = 0;
    m->capacity = 0;
    m->refcount = 1;

    struct file *f = VFS_AllocFile();
    if (!f) {
        kfree(m);
        return (uint64_t)-EMFILE;
    }

    f->path = NULL;
    f->subpath = NULL;
    f->fops = &memfd_fops;
    f->private_data = m;
    f->position = 0;
    f->refcount = 1;
    f->flags = O_RDWR;          // from hal/vfs.h
    f->socketpair_side = 0;

    int fd = VFS_AllocFd();
    if (fd < 0) {
        memfd_close(f);         // static helper in this file
        return (uint64_t)fd;
    }

    VFS_SetFd(fd, f);

    log_info("MEMFD", "sys_memfd_create -> fd=%d", fd);
    return (uint64_t)fd;
}


int memfd_ensure_capacity(memfd_t *m, size_t new_capacity)
{
    if (new_capacity <= m->capacity)
        return 0;

    size_t cap = m->capacity ? m->capacity : 4096;
    while (cap < new_capacity)
        cap *= 2;

    char *new_data = kmalloc(cap);
    if (!new_data)
        return -ENOMEM;

    if (m->data && m->size > 0) {
        for (size_t i = 0; i < m->size; ++i)
            new_data[i] = m->data[i];
        kfree(m->data);
    }

    m->data = new_data;
    m->capacity = cap;
    return 0;
}

// file_operations callbacks

static int memfd_read(struct file *f, void *buf, size_t count)
{
    log_info("MEMFD", "memfd_read count=%u", count);

    memfd_t *m = (memfd_t *)f->private_data;
    if (!m || !buf)
        return -EINVAL;

    if (f->position >= m->size)
    {
        log_info("MEMFD", "memfd_read returning 0, f->position =%u, m->size = %u ", f->position, m->size);

        return 0;  // EOF
    }

    size_t avail = m->size - f->position;
    if (count > avail)
        count = avail;

    char *dst = (char *)buf;
    char *src = m->data + f->position;

    for (size_t i = 0; i < count; ++i)
        dst[i] = src[i];

    f->position += count;
    return (int)count;
}

static int memfd_write(struct file *f, const void *buf, size_t count)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m || !buf)
        return -EINVAL;

    size_t pos = f->position;
    size_t needed = pos + count;

    if (needed > m->capacity) {
        int r = memfd_ensure_capacity(m, needed);
        if (r < 0)
            return r;
    }

    const char *src = (const char *)buf;
    char *dst = m->data + pos;

    for (size_t i = 0; i < count; ++i)
        dst[i] = src[i];

    f->position += count;
    if (needed > m->size)
        m->size = needed;

    return (int)count;
}

static int memfd_close(struct file *f)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m)
        return 0;

    m->refcount--;
    if (m->refcount == 0) {
        if (m->data)
            kfree(m->data);
        kfree(m);
    }

    f->private_data = NULL;
    return 0;
}

int memfd_stat(struct file *f, struct kstat *st)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m)
        return -EINVAL;

    st->st_dev   = 0;        // no real device backing
    st->st_ino   = 0;        // no inode number needed
    st->st_mode  = 0;        // you can set S_IFREG later if you want
    st->st_nlink = 1;        // one reference
    st->st_size  = m->size;  // THIS is what VFS_Lseek needs

    return 0;
}


// Exposed fops

struct file_operations memfd_fops = {
    .open      = NULL,
    .close     = memfd_close,
    .read      = memfd_read,
    .write     = memfd_write,
    .ioctl     = NULL,
    .readdir   = NULL,
    .stat      = memfd_stat,
    .can_read  = NULL,
    .can_write = NULL,
};
