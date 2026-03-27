// sys_event_epoll.c
#include "hal/vfs.h"
#include "errno.h"
#include "debug.h"
#include "kmalloc.h"   // kmalloc/kfree
#include <string.h>
#include <stdint.h>

/* Minimal eventfd and epoll object definitions */
struct eventfd_obj {
    uint64_t counter;
    int flags; // EFD_NONBLOCK, EFD_SEMAPHORE, etc.
};

struct epoll_watch {
    int fd;
    uint32_t events;
    uint64_t data;
};

#define MAX_EPOLL_FDS 1024

struct epoll_instance {
    struct epoll_watch *watches;
    int nfds;
};

/* Forward declarations for file ops */
static int eventfd_read(struct file *f, void *buf, size_t size);
static int eventfd_write(struct file *f, const uint8_t *buf, size_t size);
static int eventfd_close(struct file *f);

static int epoll_read(struct file *f, void *buf, size_t size);
static int epoll_write(struct file *f, const uint8_t *buf, size_t size);
static int epoll_close(struct file *f);

/* file_operations structs (minimal) */
static struct file_operations eventfd_fops = {
    .read = eventfd_read,
    .write = (int (*)(struct file*, void*, size_t))eventfd_write,
    .close = eventfd_close,
    // other ops NULL
};

static struct file_operations epoll_fops = {
    .read = epoll_read,
    .write = (int (*)(struct file*, void*, size_t))epoll_write,
    .close = epoll_close,
};

/* Helper: attach kernel object to a new fd via VFS APIs */
static int attach_object_to_fd(struct file_operations *fops, void *obj, int open_flags)
{
    int fd = VFS_AllocFd();
    if (fd < 0) {
        log_error("SYSCALL", "attach_object_to_fd: no free fd");
        return -EMFILE;
    }

    struct file *f = VFS_AllocFile();
    if (!f) {
        log_error("SYSCALL", "attach_object_to_fd: no free file objects");
        return -ENFILE;
    }

    // Initialize file object; path/subpath not needed for these pseudo-files
    f->path = NULL;
    f->subpath = NULL;
    f->fops = fops;
    f->private_data = obj;
    f->position = 0;
    f->refcount = 1;
    f->flags = open_flags;
    f->in_use = true;

    // Install into FD table
    VFS_SetFd(fd, f);
    return fd;
}

/* eventfd file ops implementation (minimal semantics) */
static int eventfd_read(struct file *f, void *buf, size_t size)
{
    if (!f || !f->private_data) return -1;
    struct eventfd_obj *e = (struct eventfd_obj *)f->private_data;
    if (size < sizeof(uint64_t)) return -1;

    // Return current counter and reset to 0 (simple semantics)
    uint64_t val = e->counter;
    // For simplicity: zero the counter on read
    e->counter = 0;

    // Copy to user buffer (VFS_Read already passes kernel buffer pointer)
    memcpy(buf, &val, sizeof(val));
    return sizeof(val);
}

static int eventfd_write(struct file *f, const uint8_t *buf, size_t size)
{
    if (!f || !f->private_data) return -1;
    if (size < sizeof(uint64_t)) return -1;
    struct eventfd_obj *e = (struct eventfd_obj *)f->private_data;
    uint64_t add = 0;
    memcpy(&add, buf, sizeof(add));
    e->counter += add;
    return sizeof(add);
}

static int eventfd_close(struct file *f)
{
    if (!f) return -1;
    if (f->private_data) {
        kfree(f->private_data);
        f->private_data = NULL;
    }
    return 0;
}

/* epoll file ops (minimal) */
static int epoll_read(struct file *f, void *buf, size_t size)
{
    // epoll_wait is implemented as a syscall; reading from the epoll fd via VFS_Read
    // is not used in this minimal implementation. Return 0 (no data).
    (void)f; (void)buf; (void)size;
    return 0;
}

static int epoll_write(struct file *f, const uint8_t *buf, size_t size)
{
    // Not used; epoll_ctl is a syscall that manipulates the epoll_instance.
    (void)f; (void)buf; (void)size;
    return -1;
}

static int epoll_close(struct file *f)
{
    if (!f) return -1;
    struct epoll_instance *epi = (struct epoll_instance *)f->private_data;
    if (epi) {
        if (epi->watches) kfree(epi->watches);
        kfree(epi);
        f->private_data = NULL;
    }
    return 0;
}

/* Helper: lookup epoll_instance from fd (returns NULL if not an epoll fd) */
struct epoll_instance *sys_epoll_from_fd(uint64_t epfd)
{
    if (epfd >= MAX_OPEN_FILES) return NULL;
    struct file *f = VFS_GetFile((int)epfd);
    if (!f) return NULL;
    if (f->fops != &epoll_fops) return NULL;
    return (struct epoll_instance *)f->private_data;
}

/* syscall: eventfd2 (290) */
long sys_eventfd2(unsigned int initval, int flags)
{
    log_info("SYSCALL", "sys_eventfd2: initval=%u flags=0x%x", initval, flags);

    struct eventfd_obj *e = (struct eventfd_obj *)kmalloc(sizeof(struct eventfd_obj));
    if (!e) {
        log_error("SYSCALL", "sys_eventfd2: kmalloc failed");
        return -ENOMEM;
    }
    e->counter = (uint64_t)initval;
    e->flags = flags;

    int fd = attach_object_to_fd(&eventfd_fops, (void *)e, 0);
    if (fd < 0) {
        kfree(e);
        return fd;
    }

    log_info("SYSCALL", "sys_eventfd2: returning fd=%d", fd);
    return fd;
}

/* syscall: epoll_create1 (291) */
long sys_epoll_create1(int flags)
{
    log_info("SYSCALL", "sys_epoll_create1: flags=0x%x", flags);

    struct epoll_instance *epi = (struct epoll_instance *)kmalloc(sizeof(struct epoll_instance));
    if (!epi) {
        log_error("SYSCALL", "sys_epoll_create1: kmalloc failed");
        return -ENOMEM;
    }

    epi->watches = (struct epoll_watch *)kmalloc(sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
    if (!epi->watches) {
        kfree(epi);
        log_error("SYSCALL", "sys_epoll_create1: kmalloc watches failed");
        return -ENOMEM;
    }
    memset(epi->watches, 0, sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
    epi->nfds = 0;

    int fd = attach_object_to_fd(&epoll_fops, (void *)epi, 0);
    if (fd < 0) {
        kfree(epi->watches);
        kfree(epi);
        return fd;
    }

    log_info("SYSCALL", "sys_epoll_create1: returning fd=%d", fd);
    return fd;
}
