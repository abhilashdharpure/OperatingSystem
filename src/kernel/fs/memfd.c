#include <stdint.h>
#include <stddef.h>
#include "memfd.h"
#include "hal/vfs.h"
#include "kmalloc.h"
#include "errno.h"
#include "debug.h"
#include "paging.h"
#include "string.h"
#include "sys_ftruncate.h"

// Helpers to grow the buffer

uint64_t sys_memfd_create(uint64_t name_ptr, uint64_t flags)
{
    (void)name_ptr;
    (void)flags;

    memfd_t *m = kmalloc(sizeof(memfd_t));
    if (!m)
        return (uint64_t)-ENOMEM;

    m->pages    = NULL;
    m->npages   = 0;
    m->size     = 0;
    m->refcount = 1;

    struct file *f = VFS_AllocFile();
    if (!f) {
        kfree(m);
        return (uint64_t)-EMFILE;
    }

    f->path           = NULL;
    f->subpath        = NULL;
    f->fops           = &memfd_fops;
    f->private_data   = m;
    f->position       = 0;
    f->refcount       = 1;
    f->flags          = O_RDWR;
    f->socketpair_side = 0;

    int fd = VFS_AllocFd();
    if (fd < 0) {
        memfd_close(f);
        return (uint64_t)fd;
    }

    VFS_SetFd(fd, f);
    log_info("MEMFD", "sys_memfd_create -> fd=%d", fd);
    return (uint64_t)fd;
}

// Grow memfd to hold at least `new_size` bytes.
// Shrinking is optional here; you can add it later.
int memfd_ensure_capacity(memfd_t *m, size_t new_size)
{
    size_t page_size  = PAGE_SIZE;
    size_t new_npages = (new_size + page_size - 1) / page_size;

    // Already large enough
    if (new_npages <= m->npages)
        return 0;

    // Allocate new page array
    uint64_t *new_pages = kmalloc(new_npages * sizeof(uint64_t));
    if (!new_pages)
        return -ENOMEM;

    // Copy existing page pointers
    for (size_t i = 0; i < m->npages; ++i)
        new_pages[i] = m->pages[i];

    // Allocate new pages
    for (size_t i = m->npages; i < new_npages; ++i) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            // Roll back newly allocated pages
            for (size_t j = m->npages; j < i; ++j)
                pmm_free_page(new_pages[j]);
            kfree(new_pages);
            return -ENOMEM;
        }

        // Zero the new page
        memset((void *)(uintptr_t)pa, 0, page_size);
        new_pages[i] = pa;
    }

    // Replace old page list
    if (m->pages)
        kfree(m->pages);

    m->pages  = new_pages;
    m->npages = new_npages;

    // Update logical size
    if (new_size > m->size)
        m->size = new_size;

    return 0;
}


// file_operations callbacks

static int memfd_read(struct file *f, void *buf, size_t count)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m || !buf)
        return -EINVAL;

    if (f->position >= m->size)
        return 0;

    size_t avail = m->size - f->position;
    if (count > avail)
        count = avail;

    size_t pos       = f->position;
    char  *dst       = buf;
    size_t remaining = count;
    size_t page_size = PAGE_SIZE;

    while (remaining > 0) {
        size_t page_idx = pos / page_size;
        size_t page_off = pos % page_size;
        size_t chunk    = page_size - page_off;
        if (chunk > remaining)
            chunk = remaining;

        uint64_t pa = m->pages[page_idx];
        char *src   = (char *)(uintptr_t)(pa + page_off);

        for (size_t i = 0; i < chunk; ++i)
            dst[i] = src[i];

        dst       += chunk;
        pos       += chunk;
        remaining -= chunk;
    }

    f->position += count;
    return (int)count;
}
static int memfd_write(struct file *f, const void *buf, size_t count)
{
    memfd_t *m = (memfd_t *)f->private_data;
    if (!m || !buf)
        return -EINVAL;

    size_t pos    = f->position;
    size_t needed = pos + count;

    if (needed > m->size) {
        int r = memfd_truncate(m, needed);
        if (r < 0)
            return r;
    }

    const char *src = buf;
    size_t remaining = count;
    size_t page_size = PAGE_SIZE;
    size_t cur = pos;

    while (remaining > 0) {
        size_t page_idx = cur / page_size;
        size_t page_off = cur % page_size;
        size_t chunk    = page_size - page_off;
        if (chunk > remaining)
            chunk = remaining;

        uint64_t pa = m->pages[page_idx];
        char *dst   = (char *)(uintptr_t)(pa + page_off);

        for (size_t i = 0; i < chunk; ++i)
            dst[i] = src[i];

        src       += chunk;
        cur       += chunk;
        remaining -= chunk;
    }

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
        if (m->pages) {
            for (size_t i = 0; i < m->npages; ++i)
                pmm_free_page(m->pages[i]);
            kfree(m->pages);
        }
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
