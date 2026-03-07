// kernel/sys/execve.c

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "hal/elf.h"
#include "kmalloc.h"
#include <boot/bootparams.h>

extern BootParams g_bootParams;
extern Process *current_process;

static void *read_entire_file(const char *path, size_t *out_size)
{
    int fd = VFS_Open(path, O_RDONLY);
    if (fd < 0) {
        log_error("EXEC", "execve: cannot open '%s'", path);
        return NULL;
    }

    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops || !f->fops->stat) {
        log_error("EXEC", "execve: no stat() for '%s'", path);
        VFS_Close(fd);
        return NULL;
    }

    struct kstat st;
    if (f->fops->stat(f, &st) < 0) {
        log_error("EXEC", "execve: stat failed for '%s'", path);
        VFS_Close(fd);
        return NULL;
    }

    size_t size = st.st_size;
    if (size == 0) {
        log_error("EXEC", "execve: file '%s' has size 0", path);
        VFS_Close(fd);
        return NULL;
    }

    void *buf = kmalloc(size);
    if (!buf) {
        log_error("EXEC", "execve: kmalloc(%llu) failed",
                  (unsigned long long)size);
        VFS_Close(fd);
        return NULL;
    }

    int n = VFS_Read(fd, buf, size);
    VFS_Close(fd);

    if (n != (int)size) {
        log_error("EXEC", "execve: read %d/%llu bytes",
                  n, (unsigned long long)size);
        kfree(buf);
        return NULL;
    }

    *out_size = size;
    return buf;
}

uint64_t sys_execve(uint64_t path_ptr,
                    uint64_t argv_ptr,
                    uint64_t envp_ptr)
{
    if (!current_process) {
        log_error("EXEC", "sys_execve: no current process");
        return (uint64_t)-1;
    }

    char kpath[256];
    const char *upath = (const char *)path_ptr;

    size_t i = 0;
    for (; i < sizeof(kpath) - 1; i++) {
        char c = upath[i];   // TODO: copy_from_user
        kpath[i] = c;
        if (c == '\0')
            break;
    }
    kpath[sizeof(kpath) - 1] = '\0';

    log_info("EXEC", "sys_execve('%s')", kpath);

    size_t elf_size = 0;
    void *elf_data = read_entire_file(kpath, &elf_size);
    if (!elf_data)
        return (uint64_t)-1;

    pid_t rc = exec_elf_mem(elf_data, elf_size, &g_bootParams);

    kfree(elf_data);

    log_error("EXEC", "sys_execve: exec_elf_mem returned unexpectedly");
    return (uint64_t)-1;
}
