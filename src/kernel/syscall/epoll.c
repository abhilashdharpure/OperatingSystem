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

#define MAX_EPOLL_FDS 64

static ssize_t epoll_read(struct file *f, void *buf, size_t len)
{
    (void)f; (void)buf; (void)len;
    return -EINVAL;
}

static ssize_t epoll_write(struct file *f, const void *buf, size_t len)
{
    (void)f; (void)buf; (void)len;
    return -EINVAL;
}


struct file_operations epoll_fops = {
    .read  = epoll_read,
    .write = epoll_write,
};

long sys_epoll_create1(int flags)
{
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

    return fd;
}



static struct epoll_instance *epoll_from_fd(int epfd_raw)
{
    log_info("SYSCALL", "sys_epoll_from_fd: looking up epfd_raw=%d", epfd_raw);

    int epfd = epfd_raw;
    struct file *f = VFS_GetFile(epfd);

    if (!f || f->fops != &epoll_fops) {
        // Try interpreting epfd_raw as a *user pointer* to an int fd
        int maybe_fd = -1;

        // If your kernel has copy_from_user, use that; otherwise, direct deref
        // if user and kernel share address space.
        maybe_fd = *(int *)(uintptr_t)epfd_raw;

        log_info("SYSCALL", "sys_epoll_from_fd: epfd_raw invalid, trying *epfd_raw=%d", maybe_fd);

        f = VFS_GetFile(maybe_fd);
        if (!f || f->fops != &epoll_fops) {
            log_info("SYSCALL", "sys_epoll_from_fd: still invalid, returning NULL");
            return NULL;
        }

        epfd = maybe_fd;
    }

    log_info("SYSCALL", "sys_epoll_from_fd: success, epfd=%d", epfd);
    return (struct epoll_instance *)f->private_data;
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

    for (int i = 0; i < epi->nfds; i++) {
        pfds[i].fd     = epi->watches[i].fd;
        pfds[i].events = 0;
        if (epi->watches[i].events & EPOLLIN)  pfds[i].events |= POLLIN;
        if (epi->watches[i].events & EPOLLOUT) pfds[i].events |= POLLOUT;
    }

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
        // you can add more mappings later

        user_events[out].events = ev;
        user_events[out].data   = epi->watches[i].data;
        out++;
    }

    return out;
}
