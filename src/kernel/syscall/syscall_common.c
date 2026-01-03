// syscall_common.c
#include "syscall_common.h"
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "pmm.h"
#include "paging.h"

#define PAGE_SIZE 0x1000

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

Process *current_process;


ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len)
{
    (void)fd;
    for (uint64_t i = 0; i < len; ++i)
    {
        serial_putc(buf[i]);
    }

    return len;
}

__attribute__((noreturn))
void sys_exit(uint64_t code)
{
    log_info("SYSCALL", "userspace exited with code %llu",
             (unsigned long long)code);

    // Instead of halting, spin with a heartbeat
    for (;;) {
        log_info("SYSCALL", "spinning after exit...");
        for (volatile uint64_t i = 0; i < 100000000; ++i) { }
    }
}

int64_t sys_open(const char *path, uint64_t flags, uint64_t mode)
{
    (void)mode; // ignore for now

    log_info("SYSCALL", "sys_open path=%s flags=%llx",
             path, (unsigned long long)flags);

    int fd = VFS_Open(path, (int)flags);
    if (fd < 0)
        return -1;

    return fd;
}

ssize_t sys_read(uint64_t fd, void *buf, uint64_t len)
{
    log_info("SYSCALL", "sys_read fd=%llu len=%llu",
             (unsigned long long)fd, (unsigned long long)len);

    return VFS_Read((int)fd, buf, (size_t)len);
}

int64_t sys_close(uint64_t fd)
{
    log_info("SYSCALL", "sys_close fd=%llu",
             (unsigned long long)fd);

    return VFS_Close((int)fd);
}

uint64_t sys_mmap(uint64_t addr,
                  uint64_t length,
                  uint64_t prot,
                  uint64_t flags,
                  uint64_t fd,
                  uint64_t offset)
{
    log_info("SYSCALL", "sys_mmap addr=%llx len=%llx prot=%llx flags=%llx fd=%lld off=%llx",
             (unsigned long long)addr,
             (unsigned long long)length,
             (unsigned long long)prot,
             (unsigned long long)flags,
             (long long)fd,
             (unsigned long long)offset);

    // Enforce our minimal subset
    if (addr != 0) {
        log_error("SYSCALL", "sys_mmap: non-zero addr not supported");
        return (uint64_t)-1; // MAP_FAILED
    }

    if ((flags & (MAP_ANONYMOUS | MAP_PRIVATE)) != (MAP_ANONYMOUS | MAP_PRIVATE)) {
        log_error("SYSCALL", "sys_mmap: only MAP_ANONYMOUS|MAP_PRIVATE supported");
        return (uint64_t)-1;
    }

    if (fd != (uint64_t)-1 || offset != 0) {
        log_error("SYSCALL", "sys_mmap: file-backed mappings not supported yet");
        return (uint64_t)-1;
    }

    if (!(prot & PROT_READ) || !(prot & PROT_WRITE)) {
        log_error("SYSCALL", "sys_mmap: only RW mappings supported for now");
        return (uint64_t)-1;
    }

    // Round length up to page size
    if (length == 0) {
        return (uint64_t)-1;
    }

    uint64_t pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;

    // Pick VA from per-process mmap_base
    uint64_t va_start = current_process->mmap_base;
    uint64_t va = va_start;
    current_process->mmap_base += pages * PAGE_SIZE;

    // Map pages
    for (uint64_t i = 0; i < pages; ++i) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            log_error("SYSCALL", "sys_mmap: out of physical memory");
            // TODO: unmap already mapped pages
            return (uint64_t)-1;
        }

        map_page(current_process->page_directory, va, pa,
                 PAGE_PRESENT | PAGE_RW | PAGE_USER);
        va += PAGE_SIZE;
    }

    log_info("SYSCALL", "sys_mmap: mapped %llu pages at 0x%llx",
             (unsigned long long)pages,
             (unsigned long long)va_start);

    return va_start;
}
