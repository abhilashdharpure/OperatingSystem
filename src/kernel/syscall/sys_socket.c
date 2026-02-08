// sys_socket.c

#include "sys_socket.h"
#include "unix_socket.h"
#include "kmalloc.h"
#include "errno.h"
#include "hal/vfs.h"
#include "string.h"
#include "socket.h"
#include "syscall/un.h"
#include "debug.h"

#define MAX_UNIX_SOCKS 64


int unix_sock_write(struct file *f, const void *buf, size_t size)
{
    unix_socket_t *us = (unix_socket_t *)f->private_data;
    if (!us || !buf)
        return -EINVAL;

    unix_socket_t *peer = us->peer;
    if (!peer)
        return -ENOTCONN;

    if (size == 0)
        return 0;

    // clamp to remaining space in peer buffer
    size_t space = sizeof(peer->buf) - peer->buf_len;
    if (space == 0)
        return -EAGAIN;   // or -ENOBUFS for now

    if (size > space)
        size = space;

    memcpy(peer->buf + peer->buf_len, buf, size);
    peer->buf_len += size;

    return (int)size;
}

int unix_sock_read(struct file *f, void *buf, size_t size)
{
    unix_socket_t *us = (unix_socket_t *)f->private_data;
    if (!us || !buf)
        return -EINVAL;

    if (us->buf_len == 0)
        return 0;   // EOF / no data yet (for now, non-blocking)

    if (size > us->buf_len)
        size = us->buf_len;

    memcpy(buf, us->buf, size);

    // shift remaining data to front
    size_t remaining = us->buf_len - size;
    if (remaining > 0)
        memmove(us->buf, us->buf + size, remaining);

    us->buf_len = remaining;
    return (int)size;
}

int unix_sock_ioctl(struct file *f, unsigned long req, void *arg)
{
    (void)f; (void)req; (void)arg;
    return -EINVAL;
}

int unix_sock_close(struct file *f)
{
    unix_socket_t *us = (unix_socket_t *)f->private_data;
    // TODO: free peers, pending, etc. For now just free self.
    if (us) {
        kfree(us);
        f->private_data = NULL;
    }
    return 0;
}


struct file_operations unix_socket_fops = {
    .open      = NULL,
    .close     = unix_sock_close,
    .read      = unix_sock_read,
    .write     = unix_sock_write,
    .ioctl     = unix_sock_ioctl,
    .readdir   = NULL,
    .stat      = NULL,
    .can_read  = NULL,
    .can_write = NULL,
};

static unix_socket_t *g_unix_socks[MAX_UNIX_SOCKS];

static void unix_register_socket(unix_socket_t *us)
{
    for (int i = 0; i < MAX_UNIX_SOCKS; ++i) {
        if (!g_unix_socks[i]) {
            g_unix_socks[i] = us;
            return;
        }
    }
}

static unix_socket_t *unix_find_by_path(const char *path)
{
    for (int i = 0; i < MAX_UNIX_SOCKS; ++i) {
        unix_socket_t *us = g_unix_socks[i];
        if (us && strcmp(us->path, path) == 0)
        {
            log_info("SYS_SOCKET", "unix_find_by_path: path=%s -> us=%p listening=%u", path, us, us->listening);
            return us;
        }
    }
    log_info("SYS_SOCKET", "unix_find_by_path: path=%s -> NULL", path);
    return NULL;
}

uint64_t sys_socket(uint64_t domain, uint64_t type, uint64_t protocol)
{
    (void)protocol;

    if (domain != AF_UNIX || type != SOCK_STREAM)
        return (uint64_t)-EINVAL;

    unix_socket_t *us = kmalloc(sizeof(unix_socket_t));
    if (!us)
        return (uint64_t)-ENOMEM;

    memset(us, 0, sizeof(*us));
    log_info("SYS_SOCKET", "sys_socket: us=%p", us);

    struct file *f = VFS_AllocFile();
    if (!f) {
        kfree(us);
        return (uint64_t)-EMFILE;
    }

    f->path            = NULL;
    f->subpath         = NULL;
    f->fops            = &unix_socket_fops;
    f->private_data    = us;
    f->position        = 0;
    f->refcount        = 1;
    f->flags           = 0;
    f->socketpair_side = 0;

    int fd = VFS_AllocFd();
    if (fd < 0) {
        unix_sock_close(f);
        return (uint64_t)fd;
    }

    VFS_SetFd(fd, f);
    return (uint64_t)fd;
}

uint64_t sys_bind(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen)
{
    struct file *f = VFS_GetFile((int)fd);
    if (!f || f->fops != &unix_socket_fops)
        return (uint64_t)-EBADF;

    unix_socket_t *us = (unix_socket_t *)f->private_data;
    log_info("SYS_SOCKET", "sys_bind: fd=%d us=%p", (int)fd, us);
    if (!us)
        return (uint64_t)-EINVAL;

    if (addrlen < sizeof(struct sockaddr_un))
        return (uint64_t)-EINVAL;

    struct sockaddr_un sun;
    memcpy(&sun, (void *)(uintptr_t)addr_ptr, sizeof(sun));

    if (sun.sun_family != AF_UNIX)
        return (uint64_t)-EINVAL;

    size_t len = strnlen(sun.sun_path, sizeof(sun.sun_path));
    if (len == 0 || len >= sizeof(us->path))
        return (uint64_t)-EINVAL;

    memcpy(us->path, sun.sun_path, len + 1);
    unix_register_socket(us);
    log_info("SYS_SOCKET", "sys_bind: registered path=%s us=%p", us->path, us);

    return 0;
}

uint64_t sys_listen(uint64_t fd, uint64_t backlog)
{
    (void)backlog; // we clamp to pending[16]

    struct file *f = VFS_GetFile((int)fd);
    if (!f || f->fops != &unix_socket_fops)
        return (uint64_t)-EBADF;

    unix_socket_t *us = (unix_socket_t *)f->private_data;
    log_info("SYS_SOCKET", "sys_listen: fd=%d us=%p before listening=%u", (int)fd, us, us ? us->listening : 0);
    if (!us)
        return (uint64_t)-EINVAL;

    us->listening = true;
    log_info("SYS_SOCKET", "sys_listen: fd=%d us=%p after listening=%u", (int)fd, us, us->listening);
    return 0;
}

uint64_t sys_connect(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen)
{
    struct file *f = VFS_GetFile((int)fd);
    if (!f || f->fops != &unix_socket_fops)
        return (uint64_t)-EBADF;

    unix_socket_t *cli = (unix_socket_t *)f->private_data;
    if (!cli)
        return (uint64_t)-EINVAL;

    if (addrlen < sizeof(struct sockaddr_un))
        return (uint64_t)-EINVAL;

    struct sockaddr_un sun;
    memcpy(&sun, (void *)(uintptr_t)addr_ptr, sizeof(sun));

    if (sun.sun_family != AF_UNIX)
        return (uint64_t)-EINVAL;

    unix_socket_t *srv = unix_find_by_path(sun.sun_path);
    if (!srv || !srv->listening)
        return (uint64_t)-ECONNREFUSED;

    unix_socket_t *srv_side = kmalloc(sizeof(unix_socket_t));
    if (!srv_side)
        return (uint64_t)-ENOMEM;
    memset(srv_side, 0, sizeof(*srv_side));

    srv_side->peer = cli;
    cli->peer      = srv_side;

    int next = (srv->pending_tail + 1) % 16;
    if (next == srv->pending_head) {
        kfree(srv_side);
        return (uint64_t)-ECONNREFUSED;
    }

    srv->pending[srv->pending_tail] = srv_side;
    srv->pending_tail = next;

    return 0;
}

uint64_t sys_accept(uint64_t fd, uint64_t addr_ptr, uint64_t addrlen_ptr)
{
    log_info("SYS_SOCKET", "sys_accept start fd = %ul addr_ptr = %ul addrlen_ptr = %ul",fd, addr_ptr, addrlen_ptr);
    (void)addr_ptr;
    (void)addrlen_ptr; // ignore peer address for now

    struct file *f = VFS_GetFile((int)fd);
    if (!f || f->fops != &unix_socket_fops)
    {
        log_info("SYS_SOCKET", "sys_accept  1 return");

        return (uint64_t)-EBADF;
    }

    unix_socket_t *srv = (unix_socket_t *)f->private_data;

    log_info("SYS_SOCKET", "sys_accept: fd=%d srv=%p listening=%u",
         (int)fd, srv, srv ? srv->listening : 0);

    if (!srv || !srv->listening)
    {
        log_info("SYS_SOCKET", "sys_accept  2 return");
        return (uint64_t)-EINVAL;
    }

    if (srv->pending_head == srv->pending_tail)
    {
        log_info("SYS_SOCKET", "sys_accept  3 return");
        return (uint64_t)-EAGAIN;
    }

    unix_socket_t *cli_side = srv->pending[srv->pending_head];
    srv->pending_head = (srv->pending_head + 1) % 16;

    struct file *f_cli = VFS_AllocFile();
    if (!f_cli)
    {
        log_info("SYS_SOCKET", "sys_accept  4 return");
        return (uint64_t)-EMFILE;
    }

    f_cli->path            = NULL;
    f_cli->subpath         = NULL;
    f_cli->fops            = &unix_socket_fops;
    f_cli->private_data    = cli_side;
    f_cli->position        = 0;
    f_cli->refcount        = 1;
    f_cli->flags           = 0;
    f_cli->socketpair_side = 0;

    int newfd = VFS_AllocFd();
    if (newfd < 0) {
        unix_sock_close(f_cli);
        log_info("SYS_SOCKET", "sys_accept  5 return");
        return (uint64_t)newfd;
    }

    VFS_SetFd(newfd, f_cli);
    log_info("SYS_SOCKET", "sys_accept  6 return");
    return (uint64_t)newfd;
}

