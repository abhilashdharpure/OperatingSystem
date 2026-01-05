#include "vfs.h"
#include "pipe.h"
#include "debug.h"
#include "fcntl.h"
#include <kmalloc.h>  // kmalloc/kfree
#include <errno.h>
#include <types.h>

static int pipe_read(struct file *f, void *buf, size_t size)
{
    pipe_t *p = (pipe_t *)f->private_data;
    log_info("PIPE", "pipe_read: f=%p p=%p size=%u count_before=%u",
             f, p, (unsigned)size, (unsigned)(p ? p->count : 0));

    if (!p || size == 0)
        return 0;

    if (p->count == 0) {
        // No data available
        if (f->flags & O_NONBLOCK) {
            log_info("PIPE", "pipe_read: empty + NONBLOCK → -EAGAIN");
            return -EAGAIN;
        }

        // Blocking mode: for now return 0 (later: sleep)
        log_info("PIPE", "pipe_read: empty + BLOCKING → 0");
        return 0;
    }

    size_t to_read = (size < p->count) ? size : p->count;
    uint8_t *dst = (uint8_t *)buf;

    for (size_t i = 0; i < to_read; ++i) {
        dst[i] = p->buffer[p->read_pos];
        p->read_pos = (p->read_pos + 1) % PIPE_BUF_SIZE;
    }

    p->count -= to_read;
    log_info("PIPE", "pipe_read: count_after=%u", (unsigned)p->count);
    return (int)to_read;
}

static int pipe_write(struct file *f, const void *buf, size_t size)
{
    pipe_t *p = (pipe_t *)f->private_data;
    log_info("PIPE", "pipe_write: f=%p p=%p size=%u count_before=%u",
             f, p, (unsigned)size, (unsigned)(p ? p->count : 0));

    if (!p || size == 0)
        return 0;

    size_t space = PIPE_BUF_SIZE - p->count;

    if (space == 0) {
        // Buffer full
        if (f->flags & O_NONBLOCK) {
            log_info("PIPE", "pipe_write: full + NONBLOCK → -EAGAIN");
            return -EAGAIN;
        }

        // Blocking mode: for now return 0 (later: sleep)
        log_info("PIPE", "pipe_write: full + BLOCKING → 0");
        return 0;
    }

    size_t to_write = (size < space) ? size : space;
    const uint8_t *src = (const uint8_t *)buf;

    for (size_t i = 0; i < to_write; ++i) {
        p->buffer[p->write_pos] = src[i];
        p->write_pos = (p->write_pos + 1) % PIPE_BUF_SIZE;
    }

    p->count += to_write;
    log_info("PIPE", "pipe_write: count_after=%u", (unsigned)p->count);
    return (int)to_write;
}

static int pipe_close(struct file *f)
{
    pipe_t *p = (pipe_t *)f->private_data;
    if (!p)
        return 0;

    p->refcount--;
    if (p->refcount == 0) {
        kfree(p);
    }
    f->private_data = NULL;
    return 0;
}

static struct file_operations pipe_fops = {
    .open   = NULL,
    .close  = pipe_close,
    .read   = pipe_read,
    .write  = pipe_write,
    .stat   = NULL,
    .readdir = NULL,
};

struct file_operations *pipe_get_fops(void)
{
    return &pipe_fops;
}