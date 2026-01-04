#include "socketpair.h"
#include "vfs.h"
#include <string.h>
#include <hal/file.h>

static int sp_read(struct file *f, uint8_t *buf, size_t size)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side; // 0 or 1

    uint8_t *inbuf  = which ? sp->buf1 : sp->buf0;
    uint32_t *head  = which ? &sp->head1 : &sp->head0;
    uint32_t *tail  = which ? &sp->tail1 : &sp->tail0;

    size_t count = (*tail - *head);
    if (count == 0)
        return 0;

    if (size > count)
        size = count;

    for (size_t i = 0; i < size; i++)
        buf[i] = inbuf[(*head + i) % SOCKETPAIR_BUF_SIZE];

    *head += size;
    return size;
}

static int sp_write(struct file *f, const uint8_t *buf, size_t size)
{
    socketpair_t *sp = f->private_data;
    int which = f->socketpair_side; // 0 or 1

    // write to the OTHER side's incoming buffer
    uint8_t *outbuf = which ? sp->buf0 : sp->buf1;
    uint32_t *head  = which ? &sp->head0 : &sp->head1;
    uint32_t *tail  = which ? &sp->tail0 : &sp->tail1;

    size_t free = SOCKETPAIR_BUF_SIZE - (*tail - *head);
    if (free == 0)
        return 0;

    if (size > free)
        size = free;

    for (size_t i = 0; i < size; i++)
        outbuf[(*tail + i) % SOCKETPAIR_BUF_SIZE] = buf[i];

    *tail += size;
    return size;
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
