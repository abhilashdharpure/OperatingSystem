#include <stdint.h>
#include "vfs.h"
#include "socketpair.h"
#include "uapi_socket.h"
#include "errno.h"   // where EINVAL, EBADF, ENOBUFS, etc. live
#include "types.h"

static int sp_fdq_push(int *q, int *head, int *tail, int fd)
{
    int next = (*tail + 1) % SP_MAX_FDS;
    if (next == *head)
        return -1; // full
    q[*tail] = fd;
    *tail = next;
    return 0;
}

static int sp_fdq_pop(int *q, int *head, int *tail, int *out_fd)
{
    if (*head == *tail)
        return -1; // empty
    *out_fd = q[*head];
    *head = (*head + 1) % SP_MAX_FDS;
    return 0;
}
static int sp_get_from_fd(int fd, socketpair_t **out_sp, int *out_side)
{
    if (!VFS_IsValidFd(fd))
        return -EBADF;

    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops)
        return -EBADF;

    // For now, only support our socketpair fops
    extern struct file_operations socketpair_fops;
    if (f->fops != &socketpair_fops)
        return -EINVAL;

    *out_sp   = (socketpair_t *)f->private_data;
    *out_side = f->socketpair_side;   // 0 or 1
    if (!*out_sp)
        return -EINVAL;

    return 0;
}

uint64_t sys_sendmsg(uint64_t fd_arg, uint64_t msg_ptr_arg, uint64_t flags_arg)
{
    int fd = (int)fd_arg;
    (void)flags_arg; // ignore MSG_* flags for now

    socketpair_t *sp = NULL;
    int side = 0;
    int ret = sp_get_from_fd(fd, &sp, &side);
    if (ret < 0)
        return (uint64_t)ret;

    struct msghdr_k *msg = (struct msghdr_k *)msg_ptr_arg;

    // 1. Data part: support single or multiple iovecs
    ssize_t total = 0;
    for (size_t i = 0; i < msg->msg_iovlen; ++i) {
        struct iovec_k *iov = &msg->msg_iov[i];
        if (!iov->iov_base || iov->iov_len == 0)
            continue;

        // Reuse your existing sp_write
        extern ssize_t sp_write(struct file *f, const void *buf, size_t count);
        struct file *f = VFS_GetFile(fd);
        ssize_t n = sp_write(f, iov->iov_base, iov->iov_len);
        if (n < 0)
            return (uint64_t)n;  // propagate -EAGAIN etc.
        total += n;
        if ((size_t)n < iov->iov_len)
            break; // partial write
    }

    // 2. Control part: SCM_RIGHTS
    if (msg->msg_control && msg->msg_controllen >= sizeof(struct cmsghdr_k)) {
        struct cmsghdr_k *cmsg = (struct cmsghdr_k *)msg->msg_control;

        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
            size_t hdr_len = CMSG_ALIGN_K(sizeof(struct cmsghdr_k));
            if (cmsg->cmsg_len < hdr_len)
                return (uint64_t)-EINVAL;

            size_t data_len = cmsg->cmsg_len - hdr_len;
            int *fds = (int *)((char *)cmsg + hdr_len);
            int nfds = (int)(data_len / sizeof(int));

            // Choose peer queue based on side
            int *q;
            int *q_head;
            int *q_tail;
            if (side == 0) {
                q = sp->fdq1;
                q_head = &sp->fdq1_head;
                q_tail = &sp->fdq1_tail;
            } else {
                q = sp->fdq0;
                q_head = &sp->fdq0_head;
                q_tail = &sp->fdq0_tail;
            }

            for (int i = 0; i < nfds; ++i) {
                int pass_fd = fds[i];

                if (!VFS_IsValidFd(pass_fd))
                    return (uint64_t)-EBADF;

                struct file *pf = VFS_GetFile(pass_fd);
                if (!pf)
                    return (uint64_t)-EBADF;

                // Bump refcount; receiver will also hold a ref
                pf->refcount++;

                if (sp_fdq_push(q, q_head, q_tail, pass_fd) < 0) {
                    // Queue full
                    return (uint64_t)-ENOBUFS;
                }
            }
        }
    }

    return (uint64_t)total;
}

uint64_t sys_recvmsg(uint64_t fd_arg, uint64_t msg_ptr_arg, uint64_t flags_arg)
{
    int fd = (int)fd_arg;
    (void)flags_arg;

    socketpair_t *sp = NULL;
    int side = 0;
    int ret = sp_get_from_fd(fd, &sp, &side);
    if (ret < 0)
        return (uint64_t)ret;

    struct msghdr_k *msg = (struct msghdr_k *)msg_ptr_arg;

    // 1. Data part
    ssize_t total = 0;
    for (size_t i = 0; i < msg->msg_iovlen; ++i) {
        struct iovec_k *iov = &msg->msg_iov[i];
        if (!iov->iov_base || iov->iov_len == 0)
            continue;

        extern ssize_t sp_read(struct file *f, void *buf, size_t count);
        struct file *f = VFS_GetFile(fd);
        ssize_t n = sp_read(f, iov->iov_base, iov->iov_len);
        if (n < 0)
            return (uint64_t)n;  // -EAGAIN, etc.

        total += n;
        if ((size_t)n < iov->iov_len)
            break; // partial
    }

    // 2. FDs via SCM_RIGHTS
    msg->msg_flags = 0;

    if (msg->msg_control && msg->msg_controllen >= sizeof(struct cmsghdr_k)) {
        // Choose our incoming FD queue
        int *q;
        int *q_head;
        int *q_tail;
        if (side == 0) {
            q = sp->fdq0;
            q_head = &sp->fdq0_head;
            q_tail = &sp->fdq0_tail;
        } else {
            q = sp->fdq1;
            q_head = &sp->fdq1_head;
            q_tail = &sp->fdq1_tail;
        }

        // How many FDs are queued?
        int available = (*q_tail - *q_head + SP_MAX_FDS) % SP_MAX_FDS;
        if (available > 0) {
            // Max FDs that fit into control buffer
            size_t max_fd_bytes = msg->msg_controllen - CMSG_ALIGN_K(sizeof(struct cmsghdr_k));
            int max_fds = (int)(max_fd_bytes / sizeof(int));
            if (max_fds <= 0) {
                // Not enough space to return even one FD
                msg->msg_controllen = 0;
                return (uint64_t)total;
            }

            int nfds = available;
            if (nfds > max_fds)
                nfds = max_fds;

            struct cmsghdr_k *cmsg = (struct cmsghdr_k *)msg->msg_control;
            size_t hdr_len = CMSG_ALIGN_K(sizeof(struct cmsghdr_k));
            int *fds = (int *)((char *)cmsg + hdr_len);

            for (int i = 0; i < nfds; ++i) {
                int fd_val;
                if (sp_fdq_pop(q, q_head, q_tail, &fd_val) < 0)
                    break;
                fds[i] = fd_val;
                // We do NOT change refcount here; sender already bumped it
            }

            size_t fd_bytes = nfds * sizeof(int);
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type  = SCM_RIGHTS;
            cmsg->cmsg_len   = CMSG_LEN_K(fd_bytes);

            msg->msg_controllen = CMSG_SPACE_K(fd_bytes);
        } else {
            msg->msg_controllen = 0;
        }
    } else {
        msg->msg_controllen = 0;
    }

    return (uint64_t)total;
}
