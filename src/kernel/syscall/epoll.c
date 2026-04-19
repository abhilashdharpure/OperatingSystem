#include "syscall/epoll.h"
#include "errno.h"
#include "kmalloc.h"
#include <string.h>
#include <stdint.h>
#include <types.h>
#include "hal/vfs.h"
#include "debug.h"
#include "hal/file.h"
#include "hal/vfs.h"
#include "kernel_poll.h"
#include "paging.h"

#define MAX_EPOLL_FDS 64

static ssize_t epoll_read(struct file *f, void *buf, size_t len)
{
    log_info("SYSCALL", "epoll_read: len =%d", len);

    (void)f; (void)buf; (void)len;
    return -EINVAL;
}

static ssize_t epoll_write(struct file *f, const void *buf, size_t len)
{
    log_info("SYSCALL", "epoll_write: len =%d", len);

    (void)f; (void)buf; (void)len;
    return -EINVAL;
}


struct file_operations epoll_fops = {
    .read  = epoll_read,
    .write = epoll_write,
};

static struct epoll_instance *epoll_from_fd(int epfd_raw)
{
    log_info("SYSCALL", "sys_epoll_from_fd: looking up epfd_raw=%d", epfd_raw);
    struct file *f = VFS_GetFile(epfd_raw);
    if (!f || f->fops != &epoll_fops)
        return NULL;
    return (struct epoll_instance *)f->private_data;
}

/* Return 1 if addr is inside the user stack range */
int is_user_stack_addr(uint64_t addr)
{
    uint64_t start = USER_START;
    uint64_t end   = USER_END;
    /* If you have current->mm stack bounds, use them here */
    return (addr >= start && addr < end);
}

/* Basic user-range check: ensure [addr, addr+len) lies in user address space */
int is_user_range_valid(uint64_t addr, size_t len)
{
    if (len == 0) return 1;
    if (addr < USER_START) return 0;
    if (addr + (uint64_t)len - 1 >= USER_END) return 0;
    return 1;
}

/* Debug wrapper around copy_to_user. Replace calls temporarily with debug_copy_to_user. */
ssize_t debug_copy_to_user(void *dst, const void *src, size_t n)
{
    uint64_t d = (uint64_t)dst;
    uint64_t dlast = d + (n ? n - 1 : 0);

    if (is_user_stack_addr(d) || is_user_stack_addr(dlast)) {
        log_info("COPYDBG", "copy_to_user -> stack dst=%#llx len=%zu", (unsigned long long)d, n);
    }

    /* Call your kernel's real copy_to_user primitive */
    return copy_to_user(dst, src, n);
}


long sys_epoll_create1(int flags)
{
    // Add at the top of the handler
    log_info("SYSCALL291", "entry sys_epoll_create1: flags=%llx", (unsigned long long)flags);


    // if (!is_user_range_valid((void*)user_ptr, user_len)) {
    //     log_info("SYSCALL291", "bad user ptr: %p len=%zu", (void*)user_ptr, user_len);
    //     return -EFAULT;
    // }

    (void)flags; // ignore EPOLL_CLOEXEC for now

    int fd = VFS_AllocFd();
    if (fd < 0)
    {
        log_info("SYSCALL", "sys_epoll_create1: returning -EMFILE");
        return -EMFILE;
    }

    struct epoll_instance *epi = kmalloc(sizeof(*epi));
    if (!epi)
    {
        log_info("SYSCALL", "sys_epoll_create1: returning -ENOMEM, failed to allocate epoll_instance");
        return -ENOMEM;
    }

    memset(epi, 0, sizeof(*epi));

    struct file *f = kmalloc(sizeof(*f));
    if (!f) {
        kfree(epi);
        log_info("SYSCALL", "sys_epoll_create1: returning -ENOMEM, failed to allocate file struct");
        return -ENOMEM;
    }

    memset(f, 0, sizeof(*f));
    f->fops = &epoll_fops;
    f->private_data = epi;

    VFS_SetFd(fd, f);
    log_info("SYSCALL", "sys_epoll_create1: returning fd=%d", fd);


    /* after VFS_SetFd(fd, f); */
    struct file *f2 = VFS_GetFile(fd);
    log_info("SYSCALL", "epoll_create1 done: fd=%d f=%p f2=%p f->fops=%p epoll_fops=%p f->private=%p",
            fd, f, f2, f ? f->fops : NULL, &epoll_fops, f ? f->private_data : NULL);

    struct epoll_instance *epi_check = NULL;
    if (f2 && f2->private_data)
        epi_check = (struct epoll_instance *)f2->private_data;

    if (!f2 || f2 != f || f2->fops != &epoll_fops || !epi_check) {
        log_error("SYSCALL", "epoll_create1: inconsistent fd table or file struct");
    }
    if (epi_check) {
        log_info("SYSCALL", "epoll_create1: epi=%p nfds=%d watches=%p", epi_check, epi_check->nfds, epi_check->watches);
    }

    /* Defensive checks: ensure kernel allocations are not in user address space */
    if ((uint64_t)epi >= USER_START && (uint64_t)epi < USER_END) {
        log_error("SYSCALL291", "epi allocated in user space: %p", epi);
        kfree(epi);
        return -ENOMEM;
    }

    /* after allocating f */
    if ((uint64_t)f >= USER_START && (uint64_t)f < USER_END) {
        log_error("SYSCALL291", "file struct allocated in user space: %p", f);
        kfree(f);
        kfree(epi);
        return -ENOMEM;
    }


    log_info("SYSCALL291", "exit sys_epoll_create1: ret=%ld", fd);

    return fd;
}

long sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event *user_ev)
{
    struct epoll_instance *epi = epoll_from_fd(epfd);
    if (!epi)
    {
        log_info("SYSCALL", "sys_epoll_ctl: returning -EBADF");
        return -EBADF;
    }

    if (op != EPOLL_CTL_DEL && !user_ev)
    {
        log_info("SYSCALL", "sys_epoll_ctl: returning -EINVAL, user_ev is NULL for op=%d", op);
        return -EINVAL;
    }

    // lazy allocate watch array
    if (!epi->watches) {
        epi->watches = kmalloc(sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
        if (!epi->watches)
        {
            log_info("SYSCALL", "sys_epoll_ctl: returning -ENOMEM, failed to allocate watch array");
            return -ENOMEM;
        }
        memset(epi->watches, 0, sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
        epi->nfds = 0;
    }

    // find existing slot
    int idx = -1;
    for (int i = 0; i < epi->nfds; i++) {
        if (epi->watches[i].fd == fd) {
            idx = i;
            break;
        }
    }

    switch (op) {
    case EPOLL_CTL_ADD:
        if (idx != -1)
        {   
            log_info("SYSCALL", "sys_epoll_ctl: returning -EEXIST, fd=%d is already in epoll set", fd);        
            return -EEXIST;
        }
        if (epi->nfds >= MAX_EPOLL_FDS)
        {
            log_info("SYSCALL", "sys_epoll_ctl: returning -ENOSPC, epoll set is full (nfds=%d)", epi->nfds);
            return -ENOSPC;
        }
        idx = epi->nfds++;
        epi->watches[idx].fd     = fd;
        epi->watches[idx].events = user_ev->events;
        epi->watches[idx].data   = user_ev->data;
        log_info("SYSCALL", "sys_epoll_ctl: added fd=%d to epoll set", fd);
        return 0;

    case EPOLL_CTL_MOD:
        if (idx == -1)
        {
            log_info("SYSCALL", "sys_epoll_ctl: returning -ENOENT, fd=%d is not in epoll set", fd);
            return -ENOENT;
        }
        epi->watches[idx].events = user_ev->events;
        epi->watches[idx].data   = user_ev->data;
        log_info("SYSCALL", "sys_epoll_ctl: modified fd=%d in epoll set", fd);
        return 0;

    case EPOLL_CTL_DEL:
        if (idx == -1)
        {
            log_info("SYSCALL", "sys_epoll_ctl: returning -ENOENT, fd=%d is not in epoll set", fd);
            return -ENOENT;
        }
        // compact array
        epi->watches[idx] = epi->watches[epi->nfds - 1];
        epi->nfds--;
        log_info("SYSCALL", "sys_epoll_ctl: deleted fd=%d from epoll set", fd);
        return 0;

    default:
        log_info("SYSCALL", "sys_epoll_ctl: returning -EINVAL, unknown op=%d", op);
        return -EINVAL;
    }
}

long sys_epoll_wait(int epfd, struct epoll_event *user_events,
                    int maxevents, int timeout)
{
    log_info("SYSCALL", "sys_epoll_wait: ");

    struct epoll_instance *epi = epoll_from_fd(epfd);
    if (!epi)
        return -EBADF;

    if (maxevents <= 0 || !user_events)
        return -EINVAL;

    if (epi->nfds == 0)
        return 0;

    if (maxevents > epi->nfds)
        maxevents = epi->nfds;

    struct pollfd pfds[MAX_EPOLL_FDS];
    struct epoll_event kev[MAX_EPOLL_FDS];

    /* build pfds[] as you already do ... */

    int n = sys_poll((uint64_t)pfds, (uint64_t)epi->nfds, (uint64_t)timeout);
    if (n <= 0)
        return n;

    int out = 0;
    for (int i = 0; i < epi->nfds && out < maxevents; i++) {
        if (!pfds[i].revents)
            continue;

        uint32_t ev = 0;
        if (pfds[i].revents & POLLIN)  ev |= EPOLLIN;
        if (pfds[i].revents & POLLOUT) ev |= EPOLLOUT;

        kev[out].events = ev;
        kev[out].data   = epi->watches[i].data;
        out++;
    }

    if (out > 0) {
        if (!is_user_range_valid((uint64_t)user_events, out * sizeof(struct epoll_event)))
            return -EFAULT;

        if (copy_to_user(user_events, kev, out * sizeof(struct epoll_event)) != 0)
            return -EFAULT;
    }

    return out;
}
