// syscall_common.c
#include "syscall_common.h"
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "pmm.h"
#include "paging.h"
#include "hal/elf.h"
#include "kernel_poll.h"

#define PAGE_SIZE 0x1000
// const uint64_t PAGE_SIZE = 0x1000;

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

Process *current_process;

extern int VFS_IsValidFd(int fd);
extern int VFS_CanRead(int fd);   // for now: return 1 for regular files
extern int VFS_CanWrite(int fd);  // for now: maybe also 1


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

uint64_t sys_munmap(uint64_t addr, uint64_t length)
{
    log_info("SYSCALL", "sys_munmap addr=%llx len=%llx",
             (unsigned long long)addr,
             (unsigned long long)length);

    if (!current_process) {
        log_error("SYSCALL", "sys_munmap: current_process is NULL");
        return (uint64_t)-1;
    }

    if (addr == 0 || length == 0) {
        log_error("SYSCALL", "sys_munmap: invalid addr/len");
        return (uint64_t)-1;
    }

    // Align addr down, length up to page boundaries
    uint64_t start = addr & ~(PAGE_SIZE - 1);
    uint64_t end   = (addr + length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t va = start; va < end; va += PAGE_SIZE)
    {
        uint64_t pa = get_mapped_phys(current_process->page_directory, va);
        if (!pa) {
            log_error("SYSCALL", "sys_munmap: VA 0x%llx not mapped",
                    (unsigned long long)va);
            continue;
        }

        // Unmap and free
        unmap_page(current_process->page_directory, va);
        pmm_free_page(pa);
    }

    return 0; // success
}

uint64_t sys_mprotect(uint64_t addr, uint64_t length, uint64_t prot)
{
    log_info("SYSCALL", "sys_mprotect addr=%llx len=%llx prot=%llx",
             (unsigned long long)addr,
             (unsigned long long)length,
             (unsigned long long)prot);

    if (!current_process) {
        log_error("SYSCALL", "sys_mprotect: current_process is NULL");
        return (uint64_t)-1;
    }

    if (addr == 0 || length == 0) {
        log_error("SYSCALL", "sys_mprotect: invalid addr/len");
        return (uint64_t)-1;
    }

    // For now, support only readable, and optionally writable
    if (!(prot & PROT_READ)) {
        log_error("SYSCALL", "sys_mprotect: pages must be readable");
        return (uint64_t)-1;
    }

    // Build PTE flags from prot
    uint64_t flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE)
        flags |= PAGE_RW;
    // You can later add PROT_EXEC handling and NX bit here.

    uint64_t start = addr & ~(PAGE_SIZE - 1);
    uint64_t end   = (addr + length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t va = start; va < end; va += PAGE_SIZE) {
        uint64_t pa = get_mapped_phys(current_process->page_directory, va);
        if (!pa) {
            log_error("SYSCALL", "sys_mprotect: VA 0x%llx not mapped",
                      (unsigned long long)va);
            continue; // or return -1 if you want strict behavior
        }

        if (set_page_flags(current_process->page_directory, va, flags) != 0) {
            log_error("SYSCALL", "sys_mprotect: failed to set flags for VA 0x%llx",
                      (unsigned long long)va);
            return (uint64_t)-1;
        }
    }

    return 0;
}

uint64_t sys_brk(uint64_t new_brk)
{
    if (!current_process) {
        log_error("SYSCALL", "sys_brk: current_process is NULL");
        return 0;
    }

    // Query current break
    if (new_brk == 0) {
        return current_process->brk_cur;
    }

    uint64_t old_brk = current_process->brk_cur;

    // Enforce heap bounds
    if (new_brk < current_process->brk_start) {
        log_error("SYSCALL", "sys_brk: new_brk below heap start");
        return old_brk;
    }

    // For now, clamp to some max (static or per-process)
    if (new_brk > USER_HEAP_END) {
        log_error("SYSCALL", "sys_brk: new_brk beyond heap limit");
        return old_brk;
    }

    // Grow
    if (new_brk > old_brk) {
        uint64_t grow_start = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint64_t grow_end   = (new_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        for (uint64_t va = grow_start; va < grow_end; va += PAGE_SIZE) {
            uint64_t pa = pmm_alloc_page();
            if (!pa) {
                log_error("SYSCALL", "sys_brk: out of physical memory");
                // Do not roll back already mapped pages for simplicity.
                current_process->brk_cur = va;
                return current_process->brk_cur;
            }

            map_page(current_process->page_directory, va, pa,
                     PAGE_PRESENT | PAGE_RW | PAGE_USER);
        }

        current_process->brk_cur = new_brk;
        if (current_process->brk_end < grow_end)
            current_process->brk_end = grow_end;
        return current_process->brk_cur;
    }

    // Shrink
    if (new_brk < old_brk) {
        uint64_t shrink_start = (new_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint64_t shrink_end   = (old_brk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        for (uint64_t va = shrink_start; va < shrink_end; va += PAGE_SIZE) {
            uint64_t pa = get_mapped_phys(current_process->page_directory, va);
            if (!pa)
                continue;

            unmap_page(current_process->page_directory, va);
            pmm_free_page(pa);
        }

        current_process->brk_cur = new_brk;
        // You can also adjust brk_end downward, but not strictly necessary.
        return current_process->brk_cur;
    }

    // new_brk == old_brk
    return old_brk;
}

uint64_t sys_poll(uint64_t ufds_ptr,
                  uint64_t nfds,
                  uint64_t timeout_ms)
{
    (void)timeout_ms; // ignore for now (non-blocking)

    log_info("SYSCALL", "sys_poll: Start");

    if (ufds_ptr == 0 || nfds == 0)
        return 0;

    // Very small upper bound for now to keep stack usage sane
    if (nfds > 64) {
        log_error("SYSCALL", "sys_poll: nfds too large (%llu)",
                  (unsigned long long)nfds);
        return (uint64_t)-1;
    }

    struct pollfd local_fds[64];

    // Copy from user
    // For now, assuming user and kernel share address space; if you later
    // enforce user/kernel VA separation, you'll need a safe copy routine.
    struct pollfd *user_fds = (struct pollfd *)ufds_ptr;
    for (uint64_t i = 0; i < nfds; ++i) {
        local_fds[i] = user_fds[i];
    }

    log_info("SYSCALL", "sys_poll: nfds = %i", nfds);

    int ready_count = 0;

    for (uint64_t i = 0; i < nfds; ++i)
    {
        struct pollfd *pfd = &local_fds[i];
        pfd->revents = 0;
        log_info("SYSCALL", "sys_poll: 1");

        if (pfd->fd < 0)
        {
            log_info("SYSCALL", "sys_poll: if (pfd->fd < 0)");
            continue;
        }

        log_info("SYSCALL", "sys_poll: 2");
        if (!VFS_IsValidFd(pfd->fd))
        {
            log_info("SYSCALL", "sys_poll: if (!VFS_IsValidFd(pfd->fd))");

            pfd->revents |= POLLNVAL;
            continue;
        }
        log_info("SYSCALL", "sys_poll: 3");
        if (pfd->events & POLLIN)
        {
            log_info("SYSCALL", "sys_poll: 4");
            if (VFS_CanRead(pfd->fd))
            {
                log_info("SYSCALL", "sys_poll: VFS_CanRead");
                pfd->revents |= POLLIN;
            }
        }

        log_info("SYSCALL", "sys_poll: 5");

        if (pfd->events & POLLOUT)
        {
            log_info("SYSCALL", "sys_poll: 6");
            if (VFS_CanWrite(pfd->fd))
            {
                log_info("SYSCALL", "sys_poll: VFS_CanWrite");
                pfd->revents |= POLLOUT;
            }
        }

        log_info("SYSCALL", "sys_poll: 7");
        if (pfd->revents != 0)
            ready_count++;
    }
    log_info("SYSCALL", "sys_poll: 8");

    // Copy back to user
    for (uint64_t i = 0; i < nfds; ++i)
    {
        user_fds[i] = local_fds[i];
    }

    log_info("SYSCALL", "sys_poll: 9");

    return (uint64_t)ready_count;
}
