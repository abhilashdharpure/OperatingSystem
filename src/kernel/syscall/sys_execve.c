// kernel/sys/execve.c

#include <syscall/sys_execve.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "hal/elf.h"
#include "kmalloc.h"
#include <boot/bootparams.h>
#include "paging.h"

extern BootParams g_bootParams;
extern Process *current_process;

bool copy_from_user_byte(uint8_t *out, const uint8_t *user_ptr)
{
    uint64_t va = (uint64_t)user_ptr;
    uint64_t va_page = va & ~(PAGE_SIZE - 1);
    uint64_t off     = va & (PAGE_SIZE - 1);

    uint64_t pa_page = get_mapped_phys(current_process->page_directory, va_page);
    if (!pa_page)
        return false;

    uint8_t *kptr = (uint8_t *)phys_to_virt(pa_page + off);
    *out = *kptr;
    return true;
}


bool copy_from_user_ptr(void *out, const void *user_ptr)
{
    uint64_t va = (uint64_t)user_ptr;
    uint64_t va_page = va & ~(PAGE_SIZE - 1);
    uint64_t off     = va & (PAGE_SIZE - 1);

    uint64_t pa_page = get_mapped_phys(current_process->page_directory, va_page);
    if (!pa_page)
        return false;

    *(uint64_t *)out = *(uint64_t *)((uint8_t *)phys_to_virt(pa_page) + off);
    return true;
}

int copy_from_user(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;

    for (size_t i = 0; i < n; ++i) {
        uint64_t va = (uint64_t)(s + i);
        uint64_t pa = get_mapped_phys(current_process->page_directory, va);
        if (!pa)
        {
            log_error("EXEC", "copy_from_user: unmapped user VA=0x%llx", va);
            return -1;
        
        }
        uint64_t off = va & (PAGE_SIZE - 1);
        d[i] = *(volatile uint8_t *)phys_to_virt(pa + off);
    }

    log_error("EXEC", "copy_from_user: return 0");

    return 0;
}


void *read_entire_file(const char *path, size_t *out_size)
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
    (void)envp_ptr; // ignore for now

    if (!current_process) {
        log_error("EXEC", "sys_execve: no current process");
        return (uint64_t)-1;
    }

    // ---- 1. Copy path from userspace ----
    char kpath[256];
    size_t i = 0;

    for (; i < sizeof(kpath) - 1; i++) {
        uint8_t c;
        if (!copy_from_user_byte(&c, (const uint8_t *)path_ptr + i)) {
            kpath[i] = '\0';
            log_error("EXEC", "sys_execve: bad path pointer");
            return (uint64_t)-1;
        }
        kpath[i] = (char)c;
        if (c == '\0')
            break;
    }
    kpath[sizeof(kpath) - 1] = '\0';

    log_info("EXEC", "sys_execve('%s')", kpath);

    // ---- 2. Copy argv[] from userspace ----
    size_t argc = 0;
    char *k_argv[MAX_EXEC_ARGS] = {0};

    if (argv_ptr) {
        while (argc < MAX_EXEC_ARGS) {
            char *u_str = NULL;
            if (!copy_from_user_ptr(&u_str, (char **)argv_ptr + argc))
                break;
            if (!u_str)
                break;

            char *buf = kmalloc(MAX_EXEC_ARG_LEN);
            if (!buf)
                break;

            size_t k = 0;
            for (; k < MAX_EXEC_ARG_LEN - 1; k++) {
                uint8_t c;
                if (!copy_from_user_byte(&c, (uint8_t*)u_str + k))
                    break;
                buf[k] = c;
                if (c == '\0')
                    break;
            }
            buf[k] = '\0';

            k_argv[argc] = buf;
            argc++;
        }
    }

    // ---- 3. Open the ELF file (no read_entire_file) ----
    int fd = VFS_Open(kpath, O_RDONLY);
    if (fd < 0) {
        log_error("EXEC", "sys_execve: cannot open '%s'", kpath);
        for (size_t j = 0; j < argc; j++) {
            if (k_argv[j]) kfree(k_argv[j]);
        }
        return (uint64_t)-1;
    }

    static char *init_envp[] = {
        "PATH=/",
        NULL
    };
    // ---- 4. Execute ELF directly from fd (streaming loader) ----
    pid_t rc = exec_elf_from_fd(fd, &g_bootParams, argc, k_argv, init_envp);

    // If exec succeeds, it never returns.
    VFS_Close(fd);
    for (size_t j = 0; j < argc; j++) {
        if (k_argv[j]) kfree(k_argv[j]);
    }

    log_error("EXEC", "sys_execve: exec_elf_from_fd returned unexpectedly (rc=%d)", rc);
    return (uint64_t)-1;
}
