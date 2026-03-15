#include "vfs.h"
#include <arch/x86_64/vga_text.h>
#include <arch/x86_64/e9.h>
#include <debug.h>
#include "pipe.h"

static int vfs_count = 0;
static struct file file_table[MAX_OPEN_FILES]; // actual file objects
static struct file *open_files[MAX_OPEN_FILES]; // fd -> file*


typedef struct vfs_entry {
    const char *path;                // mount point or device path, e.g., "/"
    struct file_operations *fops;    // filesystem/device ops
    void *private_data;              // fs or device context
} vfs_entry_t;

static vfs_entry_t vfs_table[MAX_VFS_ENTRIES];

int VFS_Write(fd_t file, uint8_t* data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;

    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
            VGA_putc(data[i]);
        return size;

    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
            e9_putc(data[i]);
        return size;

    default:
        if (file >= 0 && file < MAX_OPEN_FILES && open_files[file] != NULL) {
            struct file *f = open_files[file];
            if (f->fops && f->fops->write)
                return f->fops->write(f, data, size);
        }
        return -1;
    }
}


char *kstrdup_safe(const char *s)
{
    if (!s) return NULL;
    size_t n = 0;
    while (s[n]) ++n;
    char *dst = (char*)kmalloc(n + 1);
    if (!dst) return NULL;
    for (size_t i = 0; i < n; ++i) dst[i] = s[i];
    dst[n] = '\0';
    return dst;
}

int VFS_RegisterDevice(const char *path, struct file_operations *fops, void *private_data)
{
    if (!path || !fops) return -1;
    if (vfs_count >= MAX_VFS_ENTRIES) {
        log_error("VFS", "Max device entries reached!");
        return -1;
    }

    for (int i = 0; i < vfs_count; ++i) {
        if (strcmp(vfs_table[i].path, path) == 0) {
            log_info("VFS", "Device %s already registered (index=%d)", path, i);
            return i;
        }
    }

    char *copy = kstrdup_safe(path);
    if (!copy) { log_error("VFS", "Out of mem for path"); return -1; }

    vfs_table[vfs_count].path = copy;
    vfs_table[vfs_count].fops = fops;
    vfs_table[vfs_count].private_data = private_data;

    log_info("VFS", "Registered device: %s (index=%d)", vfs_table[vfs_count].path, vfs_count);
    vfs_count++;
    return vfs_count - 1;
}

// Replace your current VFS_Open, VFS_Read and VFS_Close with the code below.
// --- Forward declarations ---
static void compute_subpath(const char *full, const char *mount, char *out, size_t outlen);
static int find_mount_for(const char *path);

void VFS_Init(void)
{
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        open_files[i] = NULL;
        file_table[i].path = NULL;
        file_table[i].refcount = 0;
    }
}

struct file *VFS_AllocFile(void)
{
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!file_table[i].in_use) {
            file_table[i].in_use = true;
            return &file_table[i];
        }
    }
    return NULL;
}

int VFS_Open(const char *path, int flags)
{
    int mnt = find_mount_for(path);
    if (mnt < 0) {
        log_error("VFS", "Not found (no mount): %s", path);
        return -1;
    }

    char subpath[256];
    compute_subpath(path, vfs_table[mnt].path, subpath, sizeof(subpath));

    int fd = VFS_AllocFd();
    if (fd < 0) {
        log_error("VFS", "Too many open files");
        return -1;
    }

    struct file *f = VFS_AllocFile();
    if (!f) {
        log_error("VFS", "No free file objects");
        return -1;
    }

    // Allocate persistent strings
    char *saved_path = kstrdup_safe(path);
    char *saved_sub  = kstrdup_safe(subpath);
    if (!saved_path || !saved_sub) {
        log_error("VFS", "Out of memory storing paths");
        if (saved_path) kfree(saved_path);
        if (saved_sub)  kfree(saved_sub);
        return -1;
    }

    // Fill file object
    f->path = saved_path;
    f->subpath = saved_sub;
    f->fops = vfs_table[mnt].fops;
    f->private_data = vfs_table[mnt].private_data;
    f->position = 0;
    f->refcount = 1;
    f->flags = flags;   // store initial open flags

    // Call filesystem open()
    if (f->fops && f->fops->open) {
        int r = f->fops->open(f, flags);
        if (r < 0) {
            kfree(saved_path);
            kfree(saved_sub);
            f->path = NULL;
            f->subpath = NULL;
            f->fops = NULL;
            f->private_data = NULL;
            return -1;
        }
    }

    // Install into FD table
    open_files[fd] = f;
    return fd;
}


int VFS_IsValidFd(fd_t fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
    {
        log_error("VFS", "VFS_IsValidFd returning NULL");
        return 0;
    }
    return open_files[fd] != NULL;
}


int VFS_Read(fd_t fd, void *buf, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd] == NULL)
    {
        log_error("VFS", "VFS_Read invalid fd or closed");
        return -1;
    }

    struct file *file = open_files[fd];

    if (file->fops && file->fops->read)
    {
        int n = file->fops->read(file, buf, size);
        if (n < 0) {
            log_error("VFS", "Underlying read failed for fd=%d", fd);
        }
        return n;
    }

    log_error("VFS", "VFS_Read: no read op");
    return -1;
}


int VFS_Close(fd_t fd)
{
    if (!VFS_IsValidFd(fd))
        return -1;

    struct file *f = open_files[fd];
    if (!f)
        return -1;

    // Remove FD entry
    open_files[fd] = NULL;

    // Decrement refcount
    f->refcount--;
    if (f->refcount > 0)
        return 0;

    // Last reference → close underlying FS object
    if (f->fops && f->fops->close)
        f->fops->close(f);

    // Free strings
    if (f->path)    kfree(f->path);
    if (f->subpath) kfree(f->subpath);

    // Clear file object
    f->path = NULL;
    f->subpath = NULL;
    f->fops = NULL;
    f->private_data = NULL;
    f->position = 0;
    f->refcount = 0;
    f->in_use = false;

    return 0;
}


// mount a filesystem or register a device at a mount point
int VFS_Mount(const char *mount_path, struct file_operations *fops, void *ctx)
{
    if (!mount_path || !fops)
    {
        return -1;
    }

    if (vfs_count >= MAX_VFS_ENTRIES)
    {
        return -1;
    }

    // dedupe
    for (int i = 0; i < vfs_count; ++i)
    {
        if (strcmp(vfs_table[i].path, mount_path) == 0) {
            vfs_table[i].fops = fops;
            vfs_table[i].private_data = ctx;
            return i;
        }
    }

    char *copy = kstrdup_safe(mount_path);
    if (!copy)
    {
        return -1;
    }

    vfs_table[vfs_count].path = copy;
    vfs_table[vfs_count].fops = fops;
    vfs_table[vfs_count].private_data = ctx;
    log_info("VFS", "Mounted: %s (index=%d)", copy, vfs_count);
    return vfs_count++;
}

void compute_subpath(const char *full, const char *mnt, char *out, size_t outsz)
{
    // Skip identical prefix "/dev" in "/dev/null"
    size_t mlen = strlen(mnt);

    // Case 1: mountpoint is "/" → subpath = full without leading '/'
    if (strcmp(mnt, "/") == 0) {
        full++; // skip leading '/'
        strncpy(out, full, outsz);
        return;
    }

    // Case 2: normal mountpoint
    const char *sub = full + mlen;

    // If full path equals mountpoint exactly → ""
    if (*sub == '\0') {
        out[0] = '\0';
        return;
    }

    // Skip one slash ("/dev" + "/null" → "null")
    if (*sub == '/')
        sub++;

    // Ensure safe copy
    strncpy(out, sub, outsz);
    out[outsz - 1] = '\0';
}


// find longest-prefix match
static int find_mount_for(const char *path)
{
    int best = -1;
    size_t best_len = 0;
    for (int i = 0; i < vfs_count; ++i) {
        const char *mp = vfs_table[i].path;
        size_t mlen = strlen(mp);
        if (mlen == 0) continue;
        if (strncmp(path, mp, mlen) == 0) {
            // exact or prefix match, prefer longest
            if (mlen > best_len) { best = i; best_len = mlen; }
        } else if (strcmp(mp, "/") == 0) {
            // root always matches anything; fallback length 1
            if (best_len == 0) { best = i; best_len = 1; }
        }
    }
    return best;
}

static void print_entry(const dirent_t *e)
{
    log_info("VFS", "  %s (inode=%u, type=%u)", e->name, e->inode, e->type);
}

void debug_list_root(void)
{
    log_info("VFS", "Listing '/'");
    VFS_List("/", print_entry);
}

int VFS_List(const char *path, void (*callback)(const dirent_t *))
{
    int fd = VFS_Open(path, 0);
    if (fd < 0) return -1;

    struct file *dir = open_files[fd];
    if (!dir->fops || !dir->fops->readdir)
    {
        VFS_Close(fd);
        return -1;
    }

    dirent_t entry;
    while (dir->fops->readdir(dir, &entry) == 0)
    {
        callback(&entry);
    }

    VFS_Close(fd);
    return 0;
}
int VFS_CanRead(fd_t fd)
{
    if (!VFS_IsValidFd(fd))
    {
        return 0;
    }

    struct file *f = open_files[fd];

    // If the file provides a readiness callback, use it
    if (f->fops && f->fops->can_read)
    {
        return f->fops->can_read(f);
    }

    // Otherwise: readable only if it has a read op
    if (f->fops && f->fops->read)
    {
        return 1;
    }

    return 0;
}

int VFS_CanWrite(fd_t fd)
{
    if (!VFS_IsValidFd(fd))
    {
        return 0;
    }

    struct file *f = open_files[fd];

    // If the file provides a readiness callback, use it
    if (f->fops && f->fops->can_write)
    {
        return f->fops->can_write(f);
    }

    // Otherwise: writable only if it has a write op
    if (f->fops && f->fops->write)
    {
        return 1;
    }

    return 0;
}

struct file *VFS_GetFile(fd_t fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return NULL;
    return open_files[fd];
}

off_t VFS_Lseek(fd_t fd, off_t offset, int whence)
{
    if (!VFS_IsValidFd(fd))
        return (off_t)-1;

    struct file *f = VFS_GetFile(fd);
    if (!f)
        return (off_t)-1;

    // Ask filesystem for size via stat()
    struct kstat st;
    if (!f->fops || !f->fops->stat)
        return (off_t)-1;

    if (f->fops->stat(f, &st) < 0)
        return (off_t)-1;

    off_t size = (off_t)st.st_size;
    off_t cur  = (off_t)f->position;
    off_t new_pos = 0;

    switch (whence) {
    case SEEK_SET: // SEEK_SET
        new_pos = offset;
        break;
    case SEEK_CUR: // SEEK_CUR
        new_pos = cur + offset;
        break;
    case SEEK_END: // SEEK_END
        new_pos = size + offset;
        break;
    default:
        return (off_t)-1;
    }

    if (new_pos < 0)
        return (off_t)-1;

    f->position = (size_t)new_pos;
    return new_pos;
}

int VFS_AllocFd(void)
{
    for (int i = VFS_FD_USER_BASE; i < MAX_OPEN_FILES; ++i) {
        if (open_files[i] == NULL) {
            return i;
        }
    }
    return -1;
}


int VFS_Dup(fd_t oldfd)
{
    if (!VFS_IsValidFd(oldfd))
        return -1;

    struct file *oldf = VFS_GetFile(oldfd);
    if (!oldf)
        return -1;

    int newfd = VFS_AllocFd();
    if (newfd < 0)
        return -1;

    open_files[newfd] = oldf;
    oldf->refcount++;
    return newfd;
}

int VFS_Dup2(fd_t oldfd, fd_t newfd)
{
    if (!VFS_IsValidFd(oldfd))
        return -1;

    if (oldfd == newfd)
        return newfd;

    struct file *oldf = VFS_GetFile(oldfd);
    if (!oldf)
        return -1;

    // If newfd is open, close it first
    if (VFS_IsValidFd(newfd)) {
        VFS_Close(newfd);
    }

    // Make sure newfd slot exists
    if (newfd < 0 || newfd >= MAX_OPEN_FILES)
        return -1;

    open_files[newfd] = oldf;
    oldf->refcount++;
    return newfd;
}

int VFS_CreatePipe(fd_t fds[2])
{
    // Allocate pipe object
    pipe_t *p = (pipe_t *)kmalloc(sizeof(pipe_t));
    if (!p)
        return -1;

    p->read_pos  = 0;
    p->write_pos = 0;
    p->count     = 0;
    p->refcount  = 2;

    // Allocate two FDs
    int fd_read = VFS_AllocFd();
    if (fd_read < 0) {
        kfree(p);
        return -1;
    }

    // Temporarily reserve fd_read so the next VFS_AllocFd() won't reuse it
    open_files[fd_read] = (struct file *)1;  // non-NULL dummy

    int fd_write = VFS_AllocFd();
    if (fd_write < 0) {
        open_files[fd_read] = NULL;
        kfree(p);
        return -1;
    }

    // Now allocate two file objects
    struct file *fr = VFS_AllocFile();
    struct file *fw = VFS_AllocFile();
    if (!fr || !fw) {
        open_files[fd_read]  = NULL;
        open_files[fd_write] = NULL;
        kfree(p);
        return -1;
    }

    // Initialize read end
    fr->path         = NULL;
    fr->subpath      = NULL;
    fr->fops         = pipe_get_fops();  // from pipe.c
    fr->private_data = p;
    fr->position     = 0;
    fr->refcount     = 1;
    fr->flags        = O_RDONLY;

    // Initialize write end
    fw->path         = NULL;
    fw->subpath      = NULL;
    fw->fops         = pipe_get_fops();
    fw->private_data = p;
    fw->position     = 0;
    fw->refcount     = 1;
    fw->flags        = O_WRONLY;

    // Install into FD table
    open_files[fd_read]  = fr;
    open_files[fd_write] = fw;

    fds[0] = fd_read;
    fds[1] = fd_write;

    return 0;
}

void VFS_SetFd(int fd, struct file *file)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES)
        return;

    open_files[fd] = file;
}
