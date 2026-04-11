#include "syscall/eventfd.h"
#include "syscall_common.h"
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "pmm.h"
#include "paging.h"
#include "hal/elf.h"
#include "errno.h"
#include "fs/memfd.h"
#include <stdint.h>
#include <drivers/fb/fb.h>

static ssize_t eventfd_read(struct file *f, void *buf, size_t len)
{
    struct eventfd_ctx *ctx = f->private_data;

    if (len < 8)
        return -EINVAL;

    uint64_t val = ctx->counter;

    // Linux semantics: reading resets counter to 0
    ctx->counter = 0;

    *(uint64_t *)buf = val;
    return 8;
}

static ssize_t eventfd_write(struct file *f, const void *buf, size_t len)
{
    struct eventfd_ctx *ctx = f->private_data;

    if (len < 8)
        return -EINVAL;

    uint64_t add = *(const uint64_t *)buf;
    ctx->counter += add;

    return 8;
}

static int eventfd_can_read(struct file *f)
{
    struct eventfd_ctx *ctx = f->private_data;
    return (ctx->counter > 0);   // readable when non‑zero
}

static int eventfd_can_write(struct file *f)
{
    (void)f;
    return 1;                    // always writable for now
}

struct file_operations eventfd_fops = {
    .open      = NULL,
    .close     = NULL,
    .read      = eventfd_read,
    .write     = eventfd_write,
    .ioctl     = NULL,
    .readdir   = NULL,
    .stat      = NULL,
    .can_read  = eventfd_can_read,
    .can_write = eventfd_can_write,
    .mmap      = NULL,
};