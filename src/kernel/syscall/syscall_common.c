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
// #include <kernel/time.h>   // whatever you use for time / nanosleep
// #include <kernel/syscall.h>
// #include <kernel/poll.h>   // struct pollfd, POLLIN, POLLOUT, POLLNVAL

uint64_t syscall_next_rip = 0;

#define PAGE_SIZE 0x1000
#define TICKS_PER_SEC 1000             // e.g. 1ms tick

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

#define MSR_FS_BASE 0xC0000100

// linux values
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003

static uint64_t current_fs_base; // per-thread in the future


Process *current_process;

extern int VFS_IsValidFd(int fd);
extern int VFS_CanRead(int fd);   // for now: return 1 for regular files
extern int VFS_CanWrite(int fd);  // for now: maybe also 1

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

    // Ensure backing store is big enough for [offset, offset+aligned_len)
    size_t needed = offset + aligned_len;
    if (memfd_ensure_capacity(m, needed) != 0)
        return (uint64_t)-1;

    // Optionally bump logical size
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

            memset((void*)(uintptr_t)pa, 0, PAGE_SIZE);
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

    uint64_t start = current_process->mmap_base;
    uint64_t len   = (length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uint64_t flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE)
        flags |= PAGE_RW;

    for (uint64_t va = start; va < start + len; va += PAGE_SIZE) {
        uint64_t pa = pmm_alloc_page();
        if (!pa)
            return (uint64_t)-1;

        memset((void*)(uintptr_t)pa, 0, PAGE_SIZE);
        if (map_page(current_process->page_directory, va, pa, flags) != 0)
            return (uint64_t)-1;
    }

    current_process->mmap_base = start + len;
    return start;
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

    if (addr != 0) {
        log_error("SYSCALL", "sys_mmap: non-zero addr not supported");
        return (uint64_t)-1;
    }

    if (length == 0)
        return (uint64_t)-1;

    // if (!(prot & PROT_READ) || !(prot & PROT_WRITE)) {
    //     log_error("SYSCALL", "sys_mmap: only RW mappings supported for now");
    //     return (uint64_t)-1;
    // }

    bool can_read  = prot & PROT_READ;
    bool can_write = prot & PROT_WRITE;

    if (!can_read) {
        log_error("SYSCALL", "sys_mmap: pages must be readable");
        return (uint64_t)-1;
    }

    if (fd == (uint64_t)-1 && (flags & MAP_ANONYMOUS)) {
        if ((flags & MAP_PRIVATE) != MAP_PRIVATE) {
            log_error("SYSCALL", "sys_mmap: anonymous must be MAP_PRIVATE");
            return (uint64_t)-1;
        }
        return mmap_anon(length, prot);
    }

    // CASE 2: file-backed mapping (memfd)
    if (!(flags & MAP_SHARED)) {
        log_error("SYSCALL", "sys_mmap: file-backed must be MAP_SHARED for now");
        return (uint64_t)-1;
    }

    if (offset != 0) {
        log_error("SYSCALL", "sys_mmap: non-zero offset not supported yet");
        return (uint64_t)-1;
    }

    return mmap_file(length, prot, flags, (int)fd, offset);
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

uint64_t sys_stat(uint64_t user_path_ptr, uint64_t user_buf_ptr)
{
    const char *path = (const char *)user_path_ptr;
    struct kstat *user_buf = (struct kstat *)user_buf_ptr;

    int fd = VFS_Open(path, 0);
    if (fd < 0)
        return (uint64_t)-1;

    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops || !f->fops->stat) {
        VFS_Close(fd);
        return (uint64_t)-1;
    }

    struct kstat st;
    int r = f->fops->stat(f, &st);
    VFS_Close(fd);

    if (r < 0)
        return (uint64_t)-1;

    *user_buf = st;
    return 0;
}

uint64_t sys_fstat(uint64_t fd, uint64_t user_buf_ptr)
{
    struct file *f = VFS_GetFile(fd);
    if (!f || !f->fops || !f->fops->stat)
        return (uint64_t)-1;

    struct kstat *user_buf = (struct kstat *)user_buf_ptr;

    struct kstat st;
    int r = f->fops->stat(f, &st);
    if (r < 0)
        return (uint64_t)-1;

    *user_buf = st;
    return 0;
}

uint64_t sys_lseek(uint64_t fd, uint64_t offset, uint64_t whence)
{
    log_info("SYSCALL", "sys_lseek fd=%u, offset=%u, whence=%u", fd,offset, whence);

    off_t ret = VFS_Lseek((fd_t)fd, (off_t)offset, (int)whence);
    return (uint64_t)ret; // return -1 on error as usual
}

uint64_t sys_getdents(uint64_t user_path_ptr,
                      uint64_t user_buf_ptr,
                      uint64_t max_entries)
{
    const char *path = (const char *)user_path_ptr;
    struct dirent *user_buf = (struct dirent *)user_buf_ptr;

    if (!path || !user_buf || max_entries == 0)
        return (uint64_t)-1;

    int fd = VFS_Open(path, 0);
    if (fd < 0)
        return (uint64_t)-1;

    struct file *dir = VFS_GetFile(fd);
    if (!dir || !dir->fops || !dir->fops->readdir) {
        VFS_Close(fd);
        return (uint64_t)-1;
    }

    // Small fixed upper bound for now
    if (max_entries > 64)
        max_entries = 64;

    dirent_t kentry;
    struct dirent temp[64];
    uint64_t count = 0;

    while (count < max_entries &&
           dir->fops->readdir(dir, &kentry) == 0)
    {
        temp[count].d_ino  = kentry.inode;
        temp[count].d_type = kentry.type;

        // Copy name safely
        size_t i = 0;
        for (; i < NAME_MAX - 1 && kentry.name[i]; ++i)
            temp[count].d_name[i] = kentry.name[i];
        temp[count].d_name[i] = '\0';

        count++;
    }

    // Copy all collected entries to userspace
    for (uint64_t i = 0; i < count; ++i) {
        user_buf[i] = temp[i];
    }

    VFS_Close(fd);
    return count;
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

// long sys_arch_prctl(int code, unsigned long addr)
// {
//     switch (code) {
//     case ARCH_SET_FS:
//         // addr is a full 64-bit canonical user VA
//         // write it into FS base MSR
//         wrmsr(0xC0000100, (uint32_t)(addr & 0xFFFFFFFF),
//                           (uint32_t)(addr >> 32));
//         return 0;

//     default:
//         return -EINVAL;
//     }
// }

long sys_arch_prctl(long code, unsigned long addr)
{
    switch (code) {
    case ARCH_SET_FS:
        wrmsr(MSR_FS_BASE, addr);   // now correct
        return 0;
    default:
        return -EINVAL;
    }
}

