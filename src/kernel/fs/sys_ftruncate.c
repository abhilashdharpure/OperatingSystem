#include <stdint.h>
#include <stddef.h>
#include "memfd.h"
#include "hal/vfs.h"
#include "kmalloc.h"
#include "errno.h"
#include "debug.h"
#include "paging.h"

int sys_ftruncate(int fd, off_t length)
{
    struct file *f = VFS_GetFile(fd);
    if (!f)
        return -EBADF;

    // Only memfd supports truncate for now
    if (f->fops != &memfd_fops)
        return -EINVAL;

    memfd_t *m = (memfd_t *)f->private_data;
    if (!m)
        return -EINVAL;

    return memfd_truncate(m, (size_t)length);
}

int memfd_truncate(memfd_t *m, size_t new_size)
{
    size_t page_size  = PAGE_SIZE;
    size_t new_npages = (new_size + page_size - 1) / page_size;

    // grow
    if (new_npages > m->npages) {
        uint64_t *new_pages = kmalloc(new_npages * sizeof(uint64_t));
        if (!new_pages)
            return -ENOMEM;

        for (size_t i = 0; i < m->npages; ++i)
            new_pages[i] = m->pages[i];

        for (size_t i = m->npages; i < new_npages; ++i) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                for (size_t j = m->npages; j < i; ++j)
                    pmm_free_page(new_pages[j]);
                kfree(new_pages);
                return -ENOMEM;
            }
            memset((void *)(uintptr_t)pa, 0, page_size);
            new_pages[i] = pa;
        }

        if (m->pages)
            kfree(m->pages);
        m->pages  = new_pages;
        m->npages = new_npages;
    }
    // shrink
    else if (new_npages < m->npages) {
        for (size_t i = new_npages; i < m->npages; ++i)
            pmm_free_page(m->pages[i]);

        if (new_npages == 0) {
            kfree(m->pages);
            m->pages = NULL;
        } else {
            uint64_t *new_pages = kmalloc(new_npages * sizeof(uint64_t));
            if (!new_pages)
                return -ENOMEM;
            for (size_t i = 0; i < new_npages; ++i)
                new_pages[i] = m->pages[i];
            kfree(m->pages);
            m->pages = new_pages;
        }

        m->npages = new_npages;
    }

    m->size = new_size;
    return 0;
}

