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
#include <stdint.h>
#include <stdbool.h>
#include <paging.h>

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

// Check if a pointer is a valid user-space pointer
bool is_valid_user_ptr(void *ptr)
{
    if (!ptr)
        return false;

    uintptr_t addr = (uintptr_t)ptr;

    // Simple linear check for valid user-space range
    if (addr >= USER_START && addr < USER_END)
        return true;

    // Also allow user stack
    if (addr >= (USER_STACK_TOP - USER_STACK_SIZE) && addr < USER_STACK_TOP)
        return true;

    // Optionally, add heap/mmap ranges
    if (addr >= USER_HEAP_START && addr < USER_HEAP_END)
        return true;

    if (addr >= USER_MMAP_BASE)
        return true;

    return false;
}



// Return 0 on success, -1 on failure
static int copy_from_user_buf(void *dst, const void *user_ptr, size_t len)
{
    if (!dst || !user_ptr) return -1;
    if (!is_valid_user_ptr((void*)user_ptr)) return -1;

    volatile uint8_t *s = (volatile uint8_t *)user_ptr;
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < len; ++i) d[i] = s[i];
    return 0;
}



static struct epoll_instance *epoll_from_fd(uint64_t epfd)
{
    struct file *f = VFS_GetFile(epfd);
    if (!f || f->fops != &epoll_fops) {
        log_info("SYSCALL", "sys_epoll_from_fd: epfd=%d invalid, returning NULL", epfd);
        return NULL;
    }
    return (struct epoll_instance *)f->private_data;
}



long sys_epoll_ctl(uint64_t epfd, uint64_t op, uint64_t fd, struct epoll_event *user_ev)
{
    log_info("SYSCALL", "sys_epoll_ctl: epfd=%d op=%d fd=%d user_ev=%p",
             (int)epfd, (int)op, (int)fd, (void*)user_ev);

    struct epoll_instance *epi = epoll_from_fd(epfd);
    if (!epi) {
        log_info("SYSCALL", "sys_epoll_ctl: returning -EBADF");
        return -EBADF;
    }

    if (op != EPOLL_CTL_DEL && !user_ev) {
        log_info("SYSCALL", "sys_epoll_ctl: returning -EINVAL, user_ev is NULL for op=%d", (int)op);
        return -EINVAL;
    }

    if (!epi->watches) {
        epi->watches = kmalloc(sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
        if (!epi->watches) {
            log_info("SYSCALL", "sys_epoll_ctl: returning -ENOMEM, failed to allocate watch array");
            return -ENOMEM;
        }
        memset(epi->watches, 0, sizeof(struct epoll_watch) * MAX_EPOLL_FDS);
        epi->nfds = 0;
    }

    int idx = -1;
    for (int i = 0; i < epi->nfds; i++) {
        if (epi->watches[i].fd == (int)fd) { idx = i; break; }
    }

    switch (op) {
    case EPOLL_CTL_ADD: {
        if (idx != -1) return -EEXIST;
        if (epi->nfds >= MAX_EPOLL_FDS) return -ENOSPC;

        struct epoll_event ev;
        if (!is_valid_user_ptr((void*)user_ev)) return -EFAULT;
        if (copy_from_user_buf(&ev, user_ev, sizeof(ev)) != 0) return -EFAULT;

        idx = epi->nfds++;
        epi->watches[idx].fd     = (int)fd;
        epi->watches[idx].events = ev.events;
        epi->watches[idx].data   = ev.data;
        log_info("SYSCALL", "sys_epoll_ctl: added fd=%d events=0x%x", (int)fd, ev.events);
        return 0;
    }

    case EPOLL_CTL_MOD: {
        if (idx == -1) return -ENOENT;

        struct epoll_event ev;
        if (!is_valid_user_ptr((void*)user_ev)) return -EFAULT;
        if (copy_from_user_buf(&ev, user_ev, sizeof(ev)) != 0) return -EFAULT;

        epi->watches[idx].events = ev.events;
        epi->watches[idx].data   = ev.data;
        log_info("SYSCALL", "sys_epoll_ctl: modified fd=%d events=0x%x", (int)fd, ev.events);
        return 0;
    }

    case EPOLL_CTL_DEL: {
        if (idx == -1) return -ENOENT;
        epi->watches[idx] = epi->watches[epi->nfds - 1];
        epi->nfds--;
        log_info("SYSCALL", "sys_epoll_ctl: deleted fd=%d from epoll set", (int)fd);
        return 0;
    }

    default:
        return -EINVAL;
    }
}


long sys_epoll_wait(uint64_t epfd, struct epoll_event *user_events,
                    uint64_t maxevents, int timeout)
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
