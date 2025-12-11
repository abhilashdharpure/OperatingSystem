#include "vfs.h"
#include <arch/i686/vga_text.h>
#include <arch/i686/e9.h>
#include <debug.h>

static int vfs_count = 0;
static struct file open_files[MAX_OPEN_FILES];

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
        if (file >= 0 && file < MAX_OPEN_FILES && open_files[file].path) {
            struct file *f = &open_files[file];
            if (f->fops && f->fops->write)
                return f->fops->write(f, data, size);
        }
        return -1;
    }
}

char *kstrdup_safe(const char *s) {
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


int VFS_Open(const char *path, int flags)
{
    log_info("VFS", "Trying to open: %s", path);

    // find mounted filesystem
    int mnt = find_mount_for(path);
    if (mnt < 0) {
        log_error("VFS", "Not found (no mount): %s", path);
        return -1;
    }

    // compute subpath relative to mountpoint
    char subpath[256];
    compute_subpath(path, vfs_table[mnt].path, subpath, sizeof(subpath));
    log_info("VFS", "VFS_Open subpath: %s", subpath);

    // find free fd entry
    for (int fd = 0; fd < MAX_OPEN_FILES; fd++)
    {
        if (open_files[fd].path == NULL)
        {

            // save subpath inside file descriptor
            char *saved = kstrdup_safe(subpath);
            if (!saved) {
                log_error("VFS", "Out of memory storing subpath");
                return -1;
            }

            open_files[fd].path = saved;
            open_files[fd].subpath = subpath;
            open_files[fd].fops = vfs_table[mnt].fops;
            open_files[fd].private_data = vfs_table[mnt].private_data;
            open_files[fd].position = 0;

            // call filesystem’s open()
            if (open_files[fd].fops && open_files[fd].fops->open) {
                int r = open_files[fd].fops->open(&open_files[fd], flags);
                if (r < 0) {
                    kfree(saved);
                    open_files[fd].path = NULL;
                    open_files[fd].fops = NULL;
                    open_files[fd].private_data = NULL;
                    log_error("VFS", "Filesystem open() failed (%s → %s)", path, subpath);
                    return -1;
                }
            }

            log_info("VFS",
                     "Opened '%s' -> fd=%d (mnt=%s, sub=%s)",
                     path, fd, vfs_table[mnt].path, subpath);
            return fd;
        }
    }

    log_error("VFS", "Too many open files");
    return -1;
}

// int VFS_Open(const char *path, int flags)
// {
//     log_info("VFS", "Trying to open: %s", path);

//     // find the mounted filesystem (longest-prefix match)
//     int mnt = find_mount_for(path);
//     if (mnt < 0) {
//         log_error("VFS", "Not found (no mount): %s", path);
//         return -1;
//     }

//     // compute path relative to mountpoint (subpath) and store it in the file
//     char subpath[256];
//     compute_subpath(path, vfs_table[mnt].path, subpath, sizeof(subpath));
//     log_info("VFS", "VFS_Open subpath: %s", subpath);
//     // find a free fd slot
//     for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
//         if (open_files[fd].path == NULL) {
//             // store the subpath (so filesystem's open/read/write can see it)
//             char *saved = kstrdup_safe(subpath);
//             if (!saved) {
//                 log_error("VFS", "Out of memory allocating path for fd");
//                 return -1;
//             }
//             open_files[fd].path = saved;
//             open_files[fd].subpath = subpath;
//             open_files[fd].fops = vfs_table[mnt].fops;
//             open_files[fd].private_data = vfs_table[mnt].private_data;
//             open_files[fd].position = 0;

//             if (open_files[fd].fops && open_files[fd].fops->open) {
//                 // call the filesystem's open; it will receive the struct file*
//                 int r = open_files[fd].fops->open(&open_files[fd], flags);
//                 if (r < 0) {
//                     // filesystem refused open: cleanup and return error
//                     kfree(saved); // free the kstrdup'd path (kfree must be available)
//                     open_files[fd].path = NULL;
//                     open_files[fd].subpath = NULL;
//                     open_files[fd].fops = NULL;
//                     open_files[fd].private_data = NULL;
//                     log_error("VFS", "Filesystem open() failed for %s (sub=%s)", path, subpath);
//                     return -1;
//                 }
//             }

//             log_info("VFS", "Opened '%s' -> fd=%d (mnt=%s, sub=%s)", path, fd, vfs_table[mnt].path, subpath);
//             return fd;
//         }
//     }

//     log_error("VFS", "No free file descriptors");
//     return -1;
// }


// int VFS_Open(const char *path, int flags)
// {
//     log_info("VFS", "Trying to open: %s", path);

//     // find mounted filesystem
//     int mnt = find_mount_for(path);
//     if (mnt < 0) {
//         log_error("VFS", "Not found (no mount): %s", path);
//         return -1;
//     }

//     // compute subpath relative to mountpoint
//     char subpath[256];
//     compute_subpath(path, vfs_table[mnt].path, subpath, sizeof(subpath));


//     log_info("VFS", "VFS_Open subpath: %s", subpath);

//     // find free fd entry
//     for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
//         if (open_files[fd].path == NULL) {

//             // save subpath inside file descriptor
//             char *saved = kstrdup_safe(subpath);
//             if (!saved) {
//                 log_error("VFS", "Out of memory storing subpath");
//                 return -1;
//             }

//             open_files[fd].path = saved;
//             open_files[fd].subpath = subpath;
//             open_files[fd].fops = vfs_table[mnt].fops;
//             open_files[fd].private_data = vfs_table[mnt].private_data;
//             open_files[fd].position = 0;

//             // call filesystem’s open()
//             if (open_files[fd].fops && open_files[fd].fops->open) {
//                 int r = open_files[fd].fops->open(&open_files[fd], flags);
//                 if (r < 0) {
//                     kfree(saved);
//                     open_files[fd].path = NULL;
//                     open_files[fd].subpath = NULL;
//                     open_files[fd].fops = NULL;
//                     open_files[fd].private_data = NULL;
//                     log_error("VFS", "Filesystem open() failed (%s → %s)", path, subpath);
//                     return -1;
//                 }
//             }

//             log_info("VFS",
//                      "Opened '%s' -> fd=%d (mnt=%s, sub=%s)",
//                      path, fd, vfs_table[mnt].path, subpath);
//             return fd;
//         }
//     }

//     log_error("VFS", "Too many open files");
//     return -1;
// }


int VFS_Read(fd_t fd, void *buf, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
    {
        log_error("VFS", "VFS_Read invalid fd or closed");
        return -1;
    }

    struct file *file = &open_files[fd];
    // log_debug("VFS", "VFS_Read file path = %s", file->path);
    // log_debug("VFS", "VFS_Read file subpath = %s", file->subpath);
    // log_debug("VFS", "VFS_Read file position = %d", file->position);
    // log_debug("VFS", "VFS_Read file private_data = %d", file->private_data);
    // log_debug("VFS", "VFS_Read file size = %d", size);



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
    if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
    {
        return -1;
    }

    struct file *file = &open_files[fd];
    if (file->fops && file->fops->close)
    {
        file->fops->close(file);
    }

    // free the kstrdup'd path we set in VFS_Open
    if (open_files[fd].path)
    {
        kfree((void*)open_files[fd].path);
        open_files[fd].path = NULL;
    }

    open_files[fd].fops = NULL;
    open_files[fd].private_data = NULL;
    file->position = 0;
    return 0;
}


// int VFS_Open(const char *path, int flags)
// {
//     log_info("VFS", "Trying to open: %s", path);

//     for (int i = 0; i < vfs_count; i++) {
//         if (strcmp(vfs_table[i].path, path) == 0) {
//             for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
//                 if (open_files[fd].path == NULL) {
//                     open_files[fd].path = vfs_table[i].path;
//                     open_files[fd].fops = vfs_table[i].fops;
//                     open_files[fd].private_data = vfs_table[i].private_data;
//                     open_files[fd].position = 0;

//                     // if (open_files[fd].fops && open_files[fd].fops->open)
//                     //     open_files[fd].fops->open(&open_files[fd]);

//                     if (open_files[fd].fops && open_files[fd].fops->open)
//                     {
//                         open_files[fd].fops->open(&open_files[fd], flags);
//                     }


//                     log_info("VFS", "Opened '%s' -> fd=%d", path, fd);
//                     return fd;
//                 }
//             }
//             log_error("VFS", "No free file descriptors");
//             return -1;
//         }
//     }
//     log_error("VFS", "Not found: %s", path);
//     return -1;
// }

// int VFS_Read(fd_t fd, void *buf, size_t size)
// {
//     if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
//     {
//         log_error("VFS", "VFS_Read returning MAX_OPEN_FILES || open_files[fd].path == NULL");
//         return -1;

//     }

//     struct file *file = &open_files[fd];
//     if (file->fops && file->fops->read)
//     {
//         log_error("VFS", "VFS_Read returning ile->fops->read(file, buf, size)");

//         return file->fops->read(file, buf, size);
//     }

//     log_error("VFS", "VFS_Read returning -1 from end");

//     return -1;
// }

// int VFS_Close(fd_t fd)
// {
//     if (fd < 0 || fd >= MAX_OPEN_FILES || open_files[fd].path == NULL)
//         return -1;

//     struct file *file = &open_files[fd];
//     if (file->fops && file->fops->close)
//         file->fops->close(file);

//     open_files[fd].path = NULL;
//     open_files[fd].fops = NULL;
//     open_files[fd].private_data = NULL;
//     file->position = 0;
//     return 0;
// }




// static const char* kstrdup_safe(const char *s) { /* your impl */ }

// mount a filesystem or register a device at a mount point
int VFS_Mount(const char *mount_path, struct file_operations *fops, void *ctx) {
    if (!mount_path || !fops) return -1;
    if (vfs_count >= MAX_VFS_ENTRIES) return -1;

    // dedupe
    for (int i = 0; i < vfs_count; ++i) {
        if (strcmp(vfs_table[i].path, mount_path) == 0) {
            vfs_table[i].fops = fops;
            vfs_table[i].private_data = ctx;
            return i;
        }
    }

    char *copy = kstrdup_safe(mount_path);
    if (!copy) return -1;

    vfs_table[vfs_count].path = copy;
    vfs_table[vfs_count].fops = fops;
    vfs_table[vfs_count].private_data = ctx;
    log_info("VFS", "Mounted: %s (index=%d)", copy, vfs_count);
    return vfs_count++;
}



// // normalize and compute subpath: strip the mount prefix
// static void compute_subpath(const char *full, const char *mount, char *out, size_t outlen) {
//     // full like "/bin/init", mount like "/"
//     size_t mlen = strlen(mount);
//     const char *start = full;
//     if (mlen > 1 && strncmp(full, mount, mlen) == 0) start = full + mlen;
//     if (*start == '/') start++; // remove leading slash after mount
//     // copy remainder
//     size_t i = 0;
//     while (start[i] && i + 1 < outlen) { out[i] = start[i]; i++; }
//     out[i] = '\0';
// }

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
static int find_mount_for(const char *path) {
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

static void print_entry(const dirent_t *e) {
    log_info("VFS", "  %s (inode=%u, type=%u)", e->name, e->inode, e->type);
}

void debug_list_root(void) {
    log_info("VFS", "Listing '/'");
    VFS_List("/", print_entry);
}

int VFS_List(const char *path, void (*callback)(const dirent_t *))
{
    int fd = VFS_Open(path, 0);
    if (fd < 0) return -1;

    struct file *dir = &open_files[fd];
    if (!dir->fops || !dir->fops->readdir) {
        VFS_Close(fd);
        return -1;
    }

    dirent_t entry;
    while (dir->fops->readdir(dir, &entry) == 0) {
        callback(&entry);
    }

    VFS_Close(fd);
    return 0;
}
