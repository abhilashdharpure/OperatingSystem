#include "socketpair.h"
#include "vfs.h"
#include <string.h>
#include <hal/file.h>
#include <errno.h>
#include <fcntl.h>
#include <types.h>

ssize_t sp_read(struct file *f, void *buf, size_t count)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side;

    uint32_t *headp = which ? &sp->head1 : &sp->head0;
    uint32_t *tailp = which ? &sp->tail1 : &sp->tail0;
    char *data      = which ? sp->buf1   : sp->buf0;

    uint32_t head = *headp;
    uint32_t tail = *tailp;

    if (tail == head) {
        // No data
        if (f->flags & O_NONBLOCK)
            return -EAGAIN;   // <-- key change

        // Blocking mode: for now, behave like EOF (later: sleep)
        return 0;
    }

    size_t avail = tail - head;
    if (count > avail)
        count = avail;

    memcpy(buf, data + head, count);
    *headp = head + count;

    return (ssize_t)count;
}

ssize_t sp_write(struct file *f, const void *buf, size_t count)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side;

    uint32_t *headp = which ? &sp->head0 : &sp->head1;
    uint32_t *tailp = which ? &sp->tail0 : &sp->tail1;
    char *data      = which ? sp->buf0   : sp->buf1;

    uint32_t head = *headp;
    uint32_t tail = *tailp;

    size_t used  = tail - head;
    size_t space = SOCKETPAIR_BUF_SIZE - used;

    if (space == 0) {
        if (f->flags & O_NONBLOCK)
            return -EAGAIN;   // <-- key change

        // Blocking mode: for now, drop/0; later: sleep
        return 0;
    }

    if (count > space)
        count = space;

    memcpy(data + tail, buf, count);
    *tailp = tail + count;

    return (ssize_t)count;
}

static int sp_close(struct file *f)
{
    return 0;
}

static int sp_can_read(struct file *f)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side;

    uint32_t head = which ? sp->head1 : sp->head0;
    uint32_t tail = which ? sp->tail1 : sp->tail0;

    return tail > head;
}

static int sp_can_write(struct file *f)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side;

    uint32_t head = which ? sp->head0 : sp->head1;
    uint32_t tail = which ? sp->tail0 : sp->tail1;

    return (tail - head) < SOCKETPAIR_BUF_SIZE;
}


struct file_operations socketpair_fops = {
    .open = NULL,
    .close = sp_close,
    .read = sp_read,
    .write = sp_write,
    .ioctl = NULL,
    .readdir = NULL,
    .stat = NULL,
    .can_read = sp_can_read,
    .can_write = sp_can_write,
};
