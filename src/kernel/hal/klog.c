#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "hal/file.h"
#include "hal/vfs.h"

#define KLOG_SIZE (128 * 1024)                         /* power of two */
static char klog_buf[KLOG_SIZE];
static volatile uint64_t klog_total;                   /* bytes ever written */

void klog_putc(char c)                                 /* must never call a log function itself */
{
    klog_buf[klog_total & (KLOG_SIZE - 1)] = c;
    klog_total++;
}

static uint64_t klog_start(void) { return klog_total > KLOG_SIZE ? klog_total - KLOG_SIZE : 0; }

static int klog_open(struct file *f, int flags) { (void)f; (void)flags; return 0; }

static int klog_read(struct file *f, void *buf, size_t size)
{
    uint64_t total = klog_total, start = klog_start();
    uint64_t pos = start + f->position;                /* position is relative to the oldest byte still in the ring */
    if (pos >= total) return 0;                        /* end of file */
    size_t n = (size_t)(total - pos);
    if (n > size) n = size;
    for (size_t i = 0; i < n; i++) ((char *)buf)[i] = klog_buf[(pos + i) & (KLOG_SIZE - 1)];
    f->position += n;
    return (int)n;
}

static int klog_stat(struct file *f, struct kstat *st)
{
    (void)f;
    memset(st, 0, sizeof *st);
    st->st_mode = 0x8000;                              /* regular file, so the editor accepts it */
    st->st_nlink = 1;
    st->st_size = klog_total - klog_start();
    return 0;
}

static struct file_operations klog_fops = { .open = klog_open, .read = klog_read, .stat = klog_stat };

void klog_register(void)
{
    VFS_RegisterDevice("/dev/kmsg", &klog_fops, NULL);
}