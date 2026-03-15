// syscall_common.c
#include "syscall_common.h"
#include "debug.h"
#include "hal/vfs.h"
#include "hal/process.h"
#include "pmm.h"
#include "paging.h"
#include "hal/elf.h"
#include "kernel_poll.h"
#include "libc/include/dirent.h"
#include "fcntl.h"
#include <time/time.h>   // your kernel-side time header
#include <arch/x86_64/pit.h>
#include <hal/socketpair.h>
#include "errno.h"
#include "fs/memfd.h"
#include <arch/x86_64/msr.h> 
#include <stdint.h>
#include <drivers/fb/fb.h>
#include "syscall/sys_linux_dirent.h"

uint64_t syscall_next_rip = 0;

static uint64_t current_fs_base; // per-thread in the future


Process *current_process;

extern int VFS_IsValidFd(int fd);
extern int VFS_CanRead(int fd);   // for now: return 1 for regular files
extern int VFS_CanWrite(int fd);  // for now: maybe also 1

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif


ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len)
{
    if (len == 0)
        return 0;

    // Always mirror to serial for debugging
    for (uint64_t i = 0; i < len; ++i)
        serial_putc(buf[i]);

    // If it's not a real VFS fd but is one of the stdio fds,
    // pretend it succeeded (serial-only).
    if (!VFS_IsValidFd((fd_t)fd)) {
        if (fd == VFS_FD_STDIN ||
            fd == VFS_FD_STDOUT ||
            fd == VFS_FD_STDERR ||
            fd == VFS_FD_DEBUG)
        {
            return (ssize_t)len;
        }
        return -1;
    }

    int written = VFS_Write((fd_t)fd, (uint8_t *)buf, (size_t)len);
    if (written < 0)
        return -1;

    return (ssize_t)written;
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

static uint64_t mmap_file(uint64_t length, uint64_t prot, uint64_t flags,
                          int fd, uint64_t offset)
{
    struct file *f = VFS_GetFile(fd);
    if (!f || f->fops != &memfd_fops)
        return (uint64_t)-1;

    memfd_t *m = (memfd_t *)f->private_data;
    if (!m)
        return (uint64_t)-1;

    size_t page_size   = PAGE_SIZE;
    size_t aligned_len = (length + page_size - 1) & ~(page_size - 1);

    size_t needed = offset + aligned_len;
    if (memfd_ensure_capacity(m, needed) != 0)
        return (uint64_t)-1;

    if (needed > m->size)
        m->size = needed;

    uint64_t va_start = current_process->mmap_base;
    uint64_t va       = va_start;
    current_process->mmap_base += aligned_len;

    size_t start_page = offset / page_size;
    size_t start_off  = offset % page_size;

    uint64_t pte_flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE)
        pte_flags |= PAGE_RW;

    for (size_t off = 0; off < aligned_len; off += page_size) {
        size_t page_idx = start_page + (start_off + off) / page_size;
        size_t page_off = (start_off + off) % page_size;

        uint64_t pa = m->pages[page_idx];
        if (!pa) {
            pa = pmm_alloc_page();
            if (!pa)
                return (uint64_t)-1;

            memset(phys_to_virt(pa), 0, PAGE_SIZE);
            m->pages[page_idx] = pa;
        }

        map_page(current_process->page_directory,
                 va,
                 pa + page_off,
                 pte_flags);

        va += page_size;
    }

    log_info("SYSCALL",
             "mmap_file: memfd mapped %llu bytes at 0x%llx (offset=%llu)",
             (unsigned long long)aligned_len,
             (unsigned long long)va_start,
             (unsigned long long)offset);

    return va_start;
}

static uint64_t mmap_anon(uint64_t length, uint64_t prot)
{
    if (!current_process)
        return (uint64_t)-1;

    if (length == 0)
        return (uint64_t)-1;

    // Initialize mmap_base once
    if (current_process->mmap_base == 0) {
        current_process->mmap_base = USER_MMAP_BASE;   // e.g. 0x50000000
    }

    // Page-align length
    uint64_t len   = (length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    // Page-align start
    uint64_t start = (current_process->mmap_base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // Build PTE flags from prot
    uint64_t flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE)
        flags |= PAGE_RW;
    // You can later add PROT_EXEC / NX handling here.
    // NOTE: PROT_NONE is a userspace concept; with this simple scheme
    //       PROT_NONE still results in a readable page (no PAGE_RW).

    for (uint64_t va = start; va < start + len; va += PAGE_SIZE) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) {
            log_error("SYSCALL", "mmap_anon: out of physical memory");
            // Optional: roll back already-mapped pages here.
            return (uint64_t)-1;
        }

        // Zero the new page via kernel virtual address
        memset(phys_to_virt(pa), 0, PAGE_SIZE);

        if (map_page(current_process->page_directory,
                     va,
                     pa,
                     flags) != 0)
        {
            log_error("SYSCALL", "mmap_anon: map_page failed for VA 0x%llx",
                      (unsigned long long)va);
            // Optional: roll back here too.
            return (uint64_t)-1;
        }
    }

    current_process->mmap_base = start + len;

    log_info("SYSCALL", "mmap_anon: mapped %llu bytes at 0x%llx (prot=%llx)",
             (unsigned long long)len,
             (unsigned long long)start,
             (unsigned long long)prot);

    return start;
}

uint64_t mmap_memfd(uint64_t length, uint64_t prot, uint64_t flags,
                    int fd, uint64_t offset)
{
    log_error("SYSCALL", "mmap_memfd is not implemented yet, but you can keep your existing memfd mmap code.");
    return mmap_file(length, prot, flags, fd, offset);
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

    if (addr != 0)
        return (uint64_t)-1;

    if (length == 0)
        return (uint64_t)-1;

    /* Anonymous mapping */
    if (fd == (uint64_t)-1 && (flags & MAP_ANONYMOUS)) {
        // For now we don’t care if it’s MAP_PRIVATE or MAP_SHARED
        return mmap_anon(length, prot);
    }

    /* File-backed mapping */
    struct file *f = VFS_GetFile((int)fd);
    if (!f) {
        log_error("SYSCALL", "sys_mmap: invalid fd %lld", (long long)fd);
        return (uint64_t)-1;
    }

    /* If the file has a real mmap op, use it */
    if (f->fops && f->fops->mmap) {
        uint64_t va = 0;
        int r = f->fops->mmap(f, length, prot, flags, offset, &va);
        return (r == 0) ? va : (uint64_t)-1;
    }

    /* memfd special case (keep your existing behavior) */
    if (f->fops == &memfd_fops) {
        if (offset != 0) {
            log_error("SYSCALL", "sys_mmap: memfd non-zero offset not supported yet");
            return (uint64_t)-1;
        }
        // For now, ignore MAP_SHARED vs MAP_PRIVATE and just map pages
        return mmap_memfd(length, prot, flags, (int)fd, offset);
    }

    /* Generic emulation for regular files: allocate anon + read file */
    {
        // If length == 0, map whole file
        if (length == 0) {
            struct kstat kst;
            if (!f->fops || !f->fops->stat || f->fops->stat(f, &kst) < 0) {
                log_error("SYSCALL", "sys_mmap: stat failed for fd %lld", (long long)fd);
                return (uint64_t)-1;
            }
            length = kst.st_size;
            if (length == 0)
                return (uint64_t)-1;
        }

        uint64_t va = mmap_anon(length, prot);
        if (va == (uint64_t)-1)
            return (uint64_t)-1;

        off_t old = VFS_Lseek((fd_t)fd, 0, SEEK_CUR);
        VFS_Lseek((fd_t)fd, (off_t)offset, SEEK_SET);

        size_t to_read = (size_t)length;
        size_t done = 0;
        while (done < to_read) {
            ssize_t r = VFS_Read((fd_t)fd, (void *)(va + done), to_read - done);
            if (r <= 0)
                break;
            done += (size_t)r;
        }

        if (old >= 0)
            VFS_Lseek((fd_t)fd, old, SEEK_SET);

        log_info("SYSCALL", "sys_mmap: emulated file-backed mmap fd=%lld -> 0x%llx len=%llu",
                 (long long)fd,
                 (unsigned long long)va,
                 (unsigned long long)length);

        return va;
    }
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

    if (!current_process)
        return (uint64_t)-1;

    if (addr == 0 || length == 0)
        return (uint64_t)-1;


    uint64_t flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE)
        flags |= PAGE_RW;
    // ignore PROT_EXEC for now

    uint64_t start = addr & ~(PAGE_SIZE - 1);
    uint64_t end   = (addr + length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t va = start; va < end; va += PAGE_SIZE) {
        uint64_t pa = get_mapped_phys(current_process->page_directory, va);
        log_info("SYSCALL", "sys_mprotect ps = %llx -> pa=0x%llx", (unsigned long long)va, (unsigned long long)pa  );

        if (!pa) {
            log_error("SYSCALL", "sys_mprotect: VA 0x%llx not mapped",
                      (unsigned long long)va);
            continue;
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

uint64_t sys_stat(uint64_t user_path_ptr, uint64_t user_buf_ptr)
{
    const char *path = (const char *)user_path_ptr;
    struct stat *ust = (struct stat *)user_buf_ptr;

    // 1. Open the file or directory
    int fd = VFS_Open(path, 0);
    if (fd < 0)
        return (uint64_t)-1;

    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops || !f->fops->stat) {
        VFS_Close(fd);
        return (uint64_t)-1;
    }

    // 2. Kernel-side stat
    struct kstat kst;
    int r = f->fops->stat(f, &kst);
    VFS_Close(fd);

    if (r < 0)
        return (uint64_t)-1;

    // 3. Translate kstat → userspace struct stat
    memset(ust, 0, sizeof(struct stat));

    ust->st_dev   = kst.st_dev;
    ust->st_ino   = kst.st_ino;
    ust->st_nlink = kst.st_nlink;
    ust->st_size  = kst.st_size;

    // IMPORTANT: set correct mode bits
    ust->st_mode  = kst.st_mode;

    // You may also set:
    ust->st_uid = 0;
    ust->st_gid = 0;
    ust->st_rdev = 0;
    ust->st_blksize = 4096;
    ust->st_blocks  = (kst.st_size + 511) / 512;

    // timestamps (optional)
    ust->st_atime = 0;
    ust->st_mtime = 0;
    ust->st_ctime = 0;

    return 0;
}

uint64_t sys_fstat(uint64_t fd, uint64_t user_buf_ptr)
{
    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops || !f->fops->stat)
        return (uint64_t)-1;

    struct kstat kst;
    int r = f->fops->stat(f, &kst);
    if (r < 0)
        return (uint64_t)-1;

    struct stat *ust = (struct stat *)user_buf_ptr;
    memset(ust, 0, sizeof(struct stat));

    ust->st_dev   = kst.st_dev;
    ust->st_ino   = kst.st_ino;
    ust->st_nlink = kst.st_nlink;
    ust->st_size  = (int64_t)kst.st_size;
    ust->st_mode  = kst.st_mode;

    ust->st_uid = 0;
    ust->st_gid = 0;
    ust->st_rdev = 0;
    ust->st_blksize = 4096;
    ust->st_blocks  = (kst.st_size + 511) / 512;

    ust->st_atime = 0;
    ust->st_mtime = 0;
    ust->st_ctime = 0;

    return 0;
}


uint64_t sys_lseek(uint64_t fd, uint64_t offset, uint64_t whence)
{
    log_info("SYSCALL", "sys_lseek fd=%u, offset=%u, whence=%u", fd,offset, whence);

    off_t ret = VFS_Lseek((fd_t)fd, (off_t)offset, (int)whence);
    return (uint64_t)ret; // return -1 on error as usual
}

uint64_t sys_getdents(uint64_t fd_arg,
                      uint64_t user_buf_ptr,
                      uint64_t buf_size)
{
    int fd = (int)fd_arg;
    char *ubuf = (char *)user_buf_ptr;
    size_t max = (size_t)buf_size;

    log_info("SYSCALL", "sys_getdents fd=%d buf=%p size=%u",
             fd, ubuf, (unsigned)max);

    if (!VFS_IsValidFd(fd) || !ubuf || max == 0)
        return (uint64_t)-1;

    struct file *dir = VFS_GetFile(fd);
    if (!dir || !dir->fops || !dir->fops->readdir)
        return (uint64_t)-1;

    size_t written = 0;
    dirent_t kentry;

    while (1) {
        if (dir->fops->readdir(dir, &kentry) != 0)
            break;  // no more entries

        size_t namelen = strnlen(kentry.name, NAME_MAX);
        size_t reclen  = sizeof(struct linux_dirent64) + namelen + 1;

        // align to 8 bytes as Linux does
        reclen = (reclen + 7) & ~7ULL;

        if (written + reclen > max)
            break;  // no space for this entry

        struct linux_dirent64 *lde =
            (struct linux_dirent64 *)(ubuf + written);

        lde->d_ino    = kentry.inode;
        lde->d_off    = 0;              // you can fill a real offset later
        lde->d_reclen = (uint16_t)reclen;
        lde->d_type   = kentry.type;    // 0=file, 1=dir in your VFS

        memcpy(lde->d_name, kentry.name, namelen);
        lde->d_name[namelen] = '\0';

        log_info("SYSCALL", "sys_getdents READDIR: %s", lde->d_name);

        written += reclen;
    }

    return (uint64_t)written;
}


uint64_t sys_dup(uint64_t oldfd)
{
    int r = VFS_Dup((fd_t)oldfd);
    return (uint64_t)r;
}

uint64_t sys_dup2(uint64_t oldfd, uint64_t newfd)
{
    int r = VFS_Dup2((fd_t)oldfd, (fd_t)newfd);
    return (uint64_t)r;
}

uint64_t sys_fcntl(uint64_t fd, uint64_t cmd, uint64_t arg)
{
    if (!VFS_IsValidFd(fd))
        return (uint64_t)-1;   // later: -EBADF

    struct file *f = VFS_GetFile(fd);

    switch (cmd) {
    case F_GETFL:
        return (uint64_t)f->flags;

    case F_SETFL: {
        int new_flags = (int)arg;
        // Only allow changing some flags (e.g., O_NONBLOCK) if you want:
        int preserved = f->flags & ~O_NONBLOCK;
        int updated   = new_flags & O_NONBLOCK;
        f->flags = preserved | updated;
        return 0;
    }

    default:
        // For now, not supported:
        // return -EINVAL later
        return (uint64_t)-1;
    }
}


uint64_t sys_pipe(uint64_t user_fds_ptr)
{
    if (!user_fds_ptr)
        return (uint64_t)-1;

    int kfds[2];
    int r = VFS_CreatePipe(kfds);
    if (r < 0)
        return (uint64_t)-1;

    int *user_fds = (int *)user_fds_ptr;
    user_fds[0] = kfds[0];
    user_fds[1] = kfds[1];

    return 0;
}

uint64_t sys_klog(uint64_t msg_ptr)
{
    const char *s = (const char *)msg_ptr;
    if (!s)
        return 0;

    while (*s)
        serial_putc(*s++);

    return 0;
}

uint64_t sys_clock_gettime(uint64_t clk_id, uint64_t tp_user)
{
    struct timespec *tp = (struct timespec *)tp_user;
    if (!tp)
        return (uint64_t)-1;

    uint64_t ticks = pit_get_ticks();   // <-- use PIT ticks
    uint64_t pit_frequency =  pit_get_frequency();
    uint64_t sec  = ticks / pit_frequency;
    uint64_t nsec = (ticks % pit_frequency) * (1000000000ULL / pit_frequency);

    tp->tv_sec  = sec;
    tp->tv_nsec = nsec;

    return 0;
}

uint64_t sys_nanosleep(uint64_t req_ptr, uint64_t rem_ptr)
{
    log_info("SYSCALL", "sys_nanosleep");

    (void)rem_ptr;

    const struct timespec *req = (const struct timespec *)req_ptr;
    if (!req)
        return (uint64_t)-1;

    uint64_t freq = pit_get_frequency();
    uint64_t req_ns = (uint64_t)req->tv_sec * 1000000000ULL
                    + (uint64_t)req->tv_nsec;

    if (req_ns == 0)
        return 0;

    uint64_t ns_per_tick = 1000000000ULL / freq;
    uint64_t ticks_to_sleep = req_ns / ns_per_tick;
    if (ticks_to_sleep == 0)
        ticks_to_sleep = 1;

    uint64_t start  = pit_get_ticks();
    uint64_t target = start + ticks_to_sleep;

    log_info("SYSCALL", "sys_nanosleep start=%llu target=%llu",
             start, target);

    // Enable interrupts so PIT IRQ can fire
    __asm__ volatile("sti");

    while (pit_get_ticks() < target) {
        // Sleep until next interrupt (PIT, keyboard, etc.)
        __asm__ volatile("hlt");
    }

    // Optionally disable interrupts again if your syscall
    // return path expects IF=0; if not, omit this.
    // __asm__ volatile("cli");

    return 0;
}

uint64_t sys_socketpair(uint64_t domain,
                        uint64_t type,
                        uint64_t protocol,
                        uint64_t sv_ptr)
{
    log_info("SYSCALL", "sys_socketpair: sv_ptr = 0x%llx",
             (unsigned long long)sv_ptr);

    if (domain != AF_UNIX || type != SOCK_STREAM)
        return (uint64_t)-1;

    // You can ignore protocol for now
    (void)protocol;

    int *sv = (int *)sv_ptr;

    socketpair_t *sp = kmalloc(sizeof(socketpair_t));
    memset(sp, 0, sizeof(*sp));

    sp->head0 = sp->tail0 = 0;
    sp->head1 = sp->tail1 = 0;
    sp->fdq0_head = sp->fdq0_tail = 0;
    sp->fdq1_head = sp->fdq1_tail = 0;
    sp->refcount = 2; // or whatever you already used before adding FD queues


    int fd0 = VFS_AllocFd();
    if (fd0 < 0) return (uint64_t)-1;

    VFS_SetFd(fd0, (struct file*)1);

    int fd1 = VFS_AllocFd();
    if (fd1 < 0) {
        VFS_SetFd(fd0, NULL);
        return (uint64_t)-1;
    }

    log_info("SYSCALL", "sys_socketpair: fd0=%d fd1=%d", fd0, fd1);

    struct file *f0 = kmalloc(sizeof(struct file));
    struct file *f1 = kmalloc(sizeof(struct file));

    memset(f0, 0, sizeof(*f0));
    memset(f1, 0, sizeof(*f1));

    f0->fops = &socketpair_fops;
    f1->fops = &socketpair_fops;

    f0->private_data = sp;
    f1->private_data = sp;

    f0->socketpair_side = 0;
    f1->socketpair_side = 1;

    VFS_SetFd(fd0, f0);
    VFS_SetFd(fd1, f1);

    sv[0] = fd0;
    sv[1] = fd1;

    return 0;
}

long sys_arch_prctl(long code, unsigned long addr)
{
    log_info("SYSCALL", "sys_arch_prctl: code=%ld addr=%lx", code, addr);
    
    switch (code) {

    case ARCH_SET_FS:
        current_process->fs_base = addr;
        wrmsr(MSR_FS_BASE, addr);
        return 0;

    case ARCH_GET_FS:
        return current_process->fs_base;

    default:
        return -EINVAL;
    }
}

long sys_getpriority(int which, int who)
{
    log_info("SYSCALL", "sys_getpriority: which=%d who=%d, retunring hard-coded value 0", which, who);

    // For now, just pretend everyone has nice 0.
    // Linux returns a value in [-20, 19], default 0.
    (void)which;
    (void)who;
    return 0;
}

long sys_faccessat(int dirfd, const char *path, int mode, int flags)
{
    (void)dirfd;
    (void)mode;
    (void)flags;

    int fd = VFS_Open(path, 0);
    if (fd < 0)
        return -ENOENT;   // or -EACCES, but ENOENT is fine

    VFS_Close(fd);
    return 0;
}

long sys_getuid(void)
{
    return 0;
}

long sys_getgid(void)
{
    return 0;
}

long sys_geteuid(void)
{
    return 0;
}

long sys_getegid(void)
{
    return 0;
}
