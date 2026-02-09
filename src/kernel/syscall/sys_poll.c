#include "sys_poll.h"
#include "kernel_poll.h"
#include "hal/vfs.h"
#include "arch/x86_64/pit.h"
#include "debug.h"
#include <stdint.h>
#include <unix_socket.h>

// Simple monotonic ms based on PIT
static uint64_t ktime_monotonic_ms(void)
{
    uint64_t ticks = pit_get_ticks();
    uint64_t freq  = pit_get_frequency(); // Hz

    if (freq == 0)
        return 0;

    // ticks / freq = seconds
    // * 1000 = ms
    return (ticks * 1000ULL) / freq;
}
static bool unix_socket_has_pending_conn(unix_socket_t *s)
{
    return s->pending_head != s->pending_tail; // simple ring buffer non-empty
}
static int unix_can_read(struct file *f)
{
    unix_socket_t *s = (unix_socket_t *)f->private_data;

    if (s->listening) {
        // readable only if accept() would succeed
        return s->pending_head != s->pending_tail;
    }

    // connected socket: readable if buffer has data
    return s->buf_len > 0;
}

static int unix_can_write(struct file *f)
{
    unix_socket_t *s = (unix_socket_t *)f->private_data;

    if (s->listening)
        return 0; // listening sockets are never writable

    // writable if buffer has space
    return s->buf_len < sizeof(s->buf);
}


// One-shot readiness scan
static int poll_do_scan(struct pollfd *fds, uint64_t nfds)
{
    int ready_count = 0;

    for (uint64_t i = 0; i < nfds; ++i) {
        struct pollfd *pfd = &fds[i];
        pfd->revents = 0;

        if (pfd->fd < 0)
            continue;

        if (!VFS_IsValidFd(pfd->fd)) {
            pfd->revents |= POLLNVAL;
            continue;
        }

        if (pfd->events & POLLIN) {
            if (VFS_CanRead(pfd->fd)) {
                pfd->revents |= POLLIN;
            }
        }

        if (pfd->events & POLLOUT) {
            if (VFS_CanWrite(pfd->fd)) {
                pfd->revents |= POLLOUT;
            }
        }

        if (pfd->revents != 0)
            ready_count++;
    }

    return ready_count;
}

uint64_t sys_poll(uint64_t ufds_ptr,
                  uint64_t nfds,
                  uint64_t timeout_ms)
{
    if (ufds_ptr == 0 || nfds == 0)
        return 0;

    if (nfds > 64) {
        log_error("SYSCALL", "sys_poll: nfds too large (%llu)",
                  (unsigned long long)nfds);
        return (uint64_t)-1;
    }

    struct pollfd *user_fds = (struct pollfd *)ufds_ptr;
    struct pollfd local_fds[64];

    // Copy from user
    for (uint64_t i = 0; i < nfds; ++i)
        local_fds[i] = user_fds[i];

    // Non-blocking case
    if ((int64_t)timeout_ms == 0) {
        int ready = poll_do_scan(local_fds, nfds);
        for (uint64_t i = 0; i < nfds; ++i)
            user_fds[i] = local_fds[i];
        return (uint64_t)ready;
    }

    // Timeout handling
    uint64_t start_ms = 0;
    uint64_t deadline_ms = 0;
    int infinite = 0;

    if ((int64_t)timeout_ms < 0) {
        infinite = 1;
    } else {
        start_ms = ktime_monotonic_ms();
        deadline_ms = start_ms + timeout_ms;
    }


    // allow timer IRQs while we wait
    __asm__ volatile("sti");



    for (;;) {
        int ready = poll_do_scan(local_fds, nfds);
        if (ready > 0) {
            for (uint64_t i = 0; i < nfds; ++i)
                user_fds[i] = local_fds[i];
            return (uint64_t)ready;
        }

        // Check timeout
        if (!infinite) {
            uint64_t now = ktime_monotonic_ms();

            // log_info("SYSCALL", "poll: now=%llu deadline=%llu", now, deadline_ms);

            if (now >= deadline_ms) {
                for (uint64_t i = 0; i < nfds; ++i)
                    user_fds[i] = local_fds[i];
                return 0; // timeout
            }
        }

        // Sleep ~1ms using PIT
        uint64_t before = ktime_monotonic_ms();
        while (ktime_monotonic_ms() == before) {
            __asm__ volatile("pause");
        }
    }
}