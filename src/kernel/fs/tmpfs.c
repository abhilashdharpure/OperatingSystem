// fs/tmpfs.c
#include "hal/vfs.h"
#include "kmalloc.h"
#include "errno.h"
#include "fcntl.h"
#include "types.h"
#include <string.h>

#define TMPFS_MAX_NODES 64
#define S_IFREG_ 0100000
#define S_IFSOCK_ 0140000

typedef struct {
    int      used;
    char     name[64];
    uint8_t *data;
    size_t   size, cap;
    uint32_t mode;
    int      opens;
    int      unlinked;
} tnode_t;

static tnode_t nodes[TMPFS_MAX_NODES];

static tnode_t *t_find(const char *name) {
    for (int i = 0; i < TMPFS_MAX_NODES; i++)
        if (nodes[i].used && !nodes[i].unlinked && strcmp(nodes[i].name, name) == 0)
            return &nodes[i];
    return NULL;
}

static void t_free(tnode_t *n) {
    if (n->data) kfree(n->data);
    memset(n, 0, sizeof(*n));
}

static int tmpfs_open(struct file *f, int flags) {
    const char *name = f->subpath;
    if (!name || !*name) return 0;                 /* the directory itself */
    tnode_t *n = t_find(name);
    if (n && (flags & O_CREAT) && (flags & O_EXCL)) return -EEXIST;
    if (!n) {
        if (!(flags & O_CREAT)) return -ENOENT;
        if (strlen(name) >= sizeof(n->name)) return -ENAMETOOLONG;
        for (int i = 0; i < TMPFS_MAX_NODES; i++)
            if (!nodes[i].used) { n = &nodes[i]; break; }
        if (!n) return -ENOSPC;
        memset(n, 0, sizeof(*n));
        n->used = 1;
        n->mode = S_IFREG_ | 0644;
        strcpy(n->name, name);
    }
    if (flags & O_TRUNC) n->size = 0;
    n->opens++;
    f->private_data = n;
    return 0;
}

static int tmpfs_close(struct file *f) {
    tnode_t *n = f->private_data;
    if (n && --n->opens <= 0 && n->unlinked) t_free(n);
    return 0;
}

static ssize_t tmpfs_read(struct file *f, void *buf, size_t len) {
    tnode_t *n = f->private_data;
    if (!n) return -EISDIR;
    if (f->position >= n->size) return 0;
    if (len > n->size - f->position) len = n->size - f->position;
    memcpy(buf, n->data + f->position, len);
    f->position += len;
    return len;
}

static ssize_t tmpfs_write(struct file *f, const void *buf, size_t len) {
    tnode_t *n = f->private_data;
    if (!n) return -EISDIR;
    size_t end = f->position + len;
    if (end > n->cap) {
        size_t ncap = n->cap ? n->cap : 4096;
        while (ncap < end) ncap *= 2;
        uint8_t *nd = kmalloc(ncap);
        if (!nd) return -ENOMEM;
        memset(nd, 0, ncap);
        if (n->data) { memcpy(nd, n->data, n->size); kfree(n->data); }
        n->data = nd; n->cap = ncap;
    }
    memcpy(n->data + f->position, buf, len);
    f->position = end;
    if (end > n->size) n->size = end;
    return len;
}

static int tmpfs_stat(struct file *f, struct kstat *st) {
    tnode_t *n = f->private_data;
    memset(st, 0, sizeof(*st));
    st->st_nlink = 1;
    if (!n) { st->st_mode = 040755; return 0; }
    st->st_mode = n->mode;
    st->st_size = n->size;
    st->st_ino  = (uint64_t)(n - nodes) + 1;
    return 0;
}

int tmpfs_unlink(void *ctx, const char *sub) {
    (void)ctx;
    tnode_t *n = t_find(sub);
    if (!n) return -ENOENT;
    n->unlinked = 1;
    if (n->opens <= 0) t_free(n);
    return 0;
}

struct file_operations tmpfs_fops = {
    .open = tmpfs_open, .close = tmpfs_close,
    .read = tmpfs_read, .write = tmpfs_write, .stat = tmpfs_stat,
};

void tmpfs_init(void) { VFS_Mount("/tmp", &tmpfs_fops, NULL); }