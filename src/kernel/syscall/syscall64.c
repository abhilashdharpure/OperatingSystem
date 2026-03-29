#include "syscall64.h"
#include "syscall_common.h"
#include "syscall_numbers.h"
#include "debug.h"
#include "errno.h"
#include "arch/x86_64/msr.h"
#include "fs/sys_ftruncate.h"
#include "hal/process.h"
#include "sys_event_epoll.h"

extern void x64_syscall_entry(void);

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084

extern uint64_t syscall_next_rip;

int m_fd = 0;
int m_epfd = 0;

// Return true if 'addr' is a canonical user-space virtual address on x86_64.
// On x86_64 user canonical addresses have bits 47..63 == 0.
static inline bool is_user_canonical(uint64_t addr)
{
    return ((addr >> 47) == 0);
}
void x64_SYSCALL_Initialize(void)
{
    uint64_t efer = rdmsr(IA32_EFER);
    efer |= (1ull << 0);              // SCE
    wrmsr(IA32_EFER, efer);

    uint16_t kernel_cs = 0x08;
    uint16_t user_cs   = 0x1B;

    uint64_t star =
        ((uint64_t)(user_cs - 0x10) << 48) |   // user base for SYSRET (we won't use it, but it's fine)
        ((uint64_t)kernel_cs        << 32);    // kernel CS for SYSCALL

    wrmsr(IA32_STAR,  star);
    wrmsr(IA32_LSTAR, (uint64_t)&x64_syscall_entry);

    uint64_t fmask = (1ull << 9) | (1ull << 8);  // IF, TF
    wrmsr(IA32_FMASK, fmask);
}

// Return 0 on success, -1 on failure
static int copy_from_user_u32(uint32_t *dst, const void *user_ptr)
{
    if (!dst || !user_ptr) return -1;
    if (!is_valid_user_ptr((void*)user_ptr)) return -1;
    volatile uint32_t *p = (volatile uint32_t *)user_ptr;
    *dst = *p;
    return 0;
}

// Try reading a 32-bit value from user_ptr + off. Return 0 on success.
static int try_read_user_u32_at_offset(uint32_t *out, const void *user_ptr, size_t off)
{
    if (!out || !user_ptr) return -1;
    const void *addr = (const void *)((uintptr_t)user_ptr + off);
    if (!is_valid_user_ptr((void*)addr)) return -1;
    return copy_from_user_u32(out, addr);
}

// Return 0 on success, -1 on failure
static int copy_from_user_u64(uint64_t *dst, const void *user_ptr)
{
    if (!dst || !user_ptr) return -1;
    if (!is_valid_user_ptr((void*)user_ptr)) return -1;
    volatile uint64_t *p = (volatile uint64_t *)user_ptr;
    *dst = *p;
    return 0;
}

// Read 'len' bytes from user VA 'va' of process 'p' into dst. Return 0 on success, -1 on failure.
static int read_user_bytes(Process *p, uint64_t va, void *dst, size_t len)
{
    size_t copied = 0;
    uint8_t *out = (uint8_t *)dst;

    while (copied < len) {
        uint64_t cur_va = va + copied;
        uint64_t pa = get_mapped_phys(p->page_directory, cur_va);
        if (!pa) return -1;
        uint64_t page_base = pa & ~(PAGE_SIZE - 1);
        uint64_t offset = cur_va & (PAGE_SIZE - 1);
        uint8_t *kva = (uint8_t *)phys_to_virt(page_base) + offset;
        size_t chunk = PAGE_SIZE - offset;
        if (chunk > (len - copied)) chunk = len - copied;
        memcpy(out + copied, kva, chunk);
        copied += chunk;
    }
    return 0;
}

static int read_user_u32(Process *p, uint64_t va, uint32_t *out)
{
    return read_user_bytes(p, va, out, sizeof(uint32_t));
}

static int read_user_u64(Process *p, uint64_t va, uint64_t *out)
{
    return read_user_bytes(p, va, out, sizeof(uint64_t));
}


// Try offsets (in bytes) inside user_ptr and optionally follow one pointer indirection.
// Returns fd >= 0 on success, -1 on failure.
static int probe_user_fd(Process *p, uint64_t user_ptr)
{
    if (!user_ptr) return -1;

    // Quick reject: if user_ptr itself is a small integer (register case), caller should check that first.
    // Try reading 32-bit values at offsets 0,4,8,12,16
    const size_t offs[] = {0, 4, 8, 12, 16};
    uint32_t val32;

    for (size_t i = 0; i < sizeof(offs)/sizeof(offs[0]); ++i) {
        uint64_t addr = user_ptr + offs[i];
        if (read_user_u32(p, addr, &val32) == 0) {
            if ((int)val32 >= 0 && (int)val32 < MAX_OPEN_FILES && VFS_IsValidFd((int)val32)) {
                log_info("SYSCALL", "probe_user_fd: found valid fd=%u at %llx+0x%zx",
                         val32, (unsigned long long)user_ptr, offs[i]);
                return (int)val32;
            }
        }
    }

    // If the memory at user_ptr looks like a pointer, follow it and probe that target
    uint64_t maybe_ptr;
    if (read_user_u64(p, user_ptr, &maybe_ptr) == 0 && is_valid_user_ptr((void*)maybe_ptr)) {
        for (size_t i = 0; i < sizeof(offs)/sizeof(offs[0]); ++i) {
            uint64_t addr = maybe_ptr + offs[i];
            if (read_user_u32(p, addr, &val32) == 0) {
                if ((int)val32 >= 0 && (int)val32 < MAX_OPEN_FILES && VFS_IsValidFd((int)val32)) {
                    log_info("SYSCALL", "probe_user_fd: found valid fd=%u at *%llx+0x%zx",
                             val32, (unsigned long long)maybe_ptr, offs[i]);
                    return (int)val32;
                }
            }
        }
    }

    return -1;
}

void debug_syscall_regs(uint64_t nr,
                        uint64_t a0,
                        uint64_t a1,
                        uint64_t a2,
                        uint64_t a3,
                        uint64_t a4,
                        uint64_t a5,
                        uint64_t rsp_before_call)
{
    log_info("SYSCALLDBG",
             "pre-call regs: nr=%llx a0=%llx a1=%llx a2=%llx a3=%llx a4=%llx a5=%llx",
             (unsigned long long)nr,
             (unsigned long long)a0,
             (unsigned long long)a1,
             (unsigned long long)a2,
             (unsigned long long)a3,
             (unsigned long long)a4,
             (unsigned long long)a5);

    log_info("SYSCALLDBG", "pre-call rsp=%llx",
             (unsigned long long)rsp_before_call);
}



uint64_t syscall_dispatch(uint64_t nr,
                          uint64_t a0,
                          uint64_t a1,
                          uint64_t a2,
                          uint64_t a3,
                          uint64_t a4,
                          uint64_t a5)
{
    log_info("SYSCALL", "syscall_dispatch: nr=%llu a0=%llx a1=%llx a2=%llx a3=%llx a4=%llx a5=%llx",
             nr, a0, a1, a2, a3, a4, a5);

    switch (nr) {
    case SYS_write:
        return sys_write(a0, (const char *)a1, a2);

    case SYS_exit:
        sys_exit(a0);
        __builtin_unreachable();

    case SYS_open:
        return sys_open((const char *)a0, a1, a2);

    case SYS_read:
        return sys_read(a0, (void *)a1, a2);

    case SYS_close:
        return sys_close(a0);

    case SYS_mmap:
        // log_info("SYSCALL", "SYS_mmap: nr=%llu a0=%llx a1=%llx a2=%llx a3=%llx a4=%llx a5=%llx",
        //      (unsigned long long)nr,
        //      (unsigned long long)a0,
        //      (unsigned long long)a1,
        //      (unsigned long long)a2,
        //      (unsigned long long)a3,
        //      (unsigned long long)a4,
        //      (unsigned long long)a5);
        return sys_mmap(a0, a1, a2, a3, a4, a5);

    case SYS_munmap:
        return sys_munmap(a0, a1);

    case SYS_mprotect:
        return sys_mprotect(a0, a1, a2);

    case SYS_brk:
        return sys_brk(a0);
    
    case SYS_poll:
        return sys_poll(a0, a1, a2);

    case SYS_stat:
        return sys_stat(a0, a1);

    case SYS_fstat:
        return sys_fstat(a0, a1);

    case SYS_lseek:
        return sys_lseek(a0, a1, a2);

    case SYS_getdents:
       return sys_getdents(a0, a1, a2);

    case SYS_dup:
        return sys_dup(a0);

    case SYS_dup2:
        return sys_dup2(a0, a1);

    case SYS_fcntl:
        m_epfd = sys_fcntl(a0, a1, a2);
        return m_epfd;

    case SYS_pipe:
        return sys_pipe(a0);

    case SYS_klog:
        return sys_klog(a0);

    case SYS_clock_gettime:
        return sys_clock_gettime(a0, a1);

    case SYS_nanosleep:
        return sys_nanosleep(a0, a1);

    case SYS_socketpair:
        log_info("SYSCALL", "SYS_socketpair: nr=%llu a0=%llx a1=%llx a2=%llx a3=%llx",
            (unsigned long long)nr,
            (unsigned long long)a0,
            (unsigned long long)a1,
            (unsigned long long)a2,
            (unsigned long long)a3);
        return sys_socketpair((int)a0, (int)a1, (int)a2, (int *)a3);

    case SYS_sendmsg:
        return sys_sendmsg(a0, a1, a2);

    case SYS_recvmsg:
        return sys_recvmsg(a0, a1, a2);

    case SYS_memfd_create:
        return sys_memfd_create(a0, a1);

    case SYS_ftruncate:
        return sys_ftruncate(a0, a1);

    case SYS_getpid: return sys_getpid();
    case SYS_getppid: return sys_getppid();
    case SYS_uname: return sys_uname((struct utsname*)a0);
    case SYS_getcwd: return sys_getcwd((char*)a0, a1);
    case SYS_madvise: return sys_madvise((void*)a0, a1, a2);


    // case SYS_set_tid_address: return sys_set_tid_address((int*)a0);
    case SYS_set_tid_address: {
        int ret = sys_set_tid_address((int*)a0);
        log_info("SYSCALL", "set_tid_address ret=%d", ret);

        // // TEMP HACK: force user RIP to main()
        // // We know from objdump that main is at 0x4000098f
        // current_process->regs.rip = 0x4000098f;
        return ret;   // <-- let userspace resume
    }


    case SYS_prlimit64: return sys_prlimit64(a0, a1, (void*)a2, (void*)a3);
    case SYS_getrandom: return sys_getrandom((void*)a0, a1, a2);
    case SYS_exit_group: return sys_exit_group(a0);

    case SYS_arch_prctl:
        // int ret = sys_arch_prctl(a0, a1);
        // log_info("SYSCALL", "sys_arch_prctl returned %d", ret);
        // // uint64_t efer = rdmsr(MSR_FS_BASE);
        // // log_info("SYSCALL", "sys_arch_prctl: EFER=0x%llx", efer);
        // // log_info("SYSCALL", "FS_BASE now = 0x%lx", rdmsr(MSR_FS_BASE));

        // return ret;

        return sys_arch_prctl(a0, a1);

     case SYS_ioctl: 
        return sys_ioctl((int)a0, (unsigned long)a1, (unsigned long)a2);

    case SYS_writev:
        return sys_writev((int)a0, (const struct iovec*)a1, (int)a2);

    case SYS_socket:
        return sys_socket(a0, a1, a2);
    case SYS_bind:
        return sys_bind(a0, a1, a2);
    case SYS_listen:
        return sys_listen(a0, a1);
    case SYS_accept:
        return sys_accept(a0, a1, a2);
    case SYS_connect:
        return sys_connect(a0, a1, a2);

    case SYS_execve:
        return sys_execve(a0, a1, a2);

    case  SYS_getpriority:
        return sys_getpriority((int)a0, (int)a1);

    case SYS_getuid:  
        log_info("SYSCALL", "SYS_getuid, returnign hardcoded 0 for now");
        return sys_getuid();

    case SYS_getgid:  
        log_info("SYSCALL", "SYS_getgid, returnign hardcoded 0 for now");
        return sys_getgid();

    case SYS_geteuid: 
        log_info("SYSCALL", "SYS_geteuid, returnign hardcoded 0 for now");
        return sys_geteuid();

    case SYS_getegid: 
        log_info("SYSCALL", "SYS_getegid, returnign hardcoded 0 for now");
        return sys_getegid();


    case SYS_faccessat:
        return sys_faccessat((int)a0, (const char *)a1, (int)a2, (int)a3);

    case SYS_faccessat2:
        log_error("SYSCALL", "Tell libc this syscall is not implemented, please fall back");
        // Tell libc "this syscall is not implemented, please fall back"
        return -ENOSYS;

    case SYS_rt_sigaction:  
        return sys_rt_sigaction((int)a0,
                                      (const struct sigaction*)a1,
                                      (struct sigaction*)a2,
                                      (size_t)a3);
    case SYS_rt_sigprocmask: 
        return sys_rt_sigprocmask((int)a0,
                                        (const sigset_t*)a1,
                                        (sigset_t*)a2,
                                        (size_t)a3);
    case SYS_clone: 
        return sys_clone((unsigned long)a0,
                               (void*)a1,
                               (void*)a2,
                               (void*)a3,
                               (void*)a4);
    case SYS_tkill:
        return sys_tkill((int)a0, (int)a1);

    case SYS_eventfd2:
        return sys_eventfd2((unsigned int)a0, (int)a1);

    case SYS_epoll_create1:
        m_fd = sys_epoll_create1(a0);          // a0 = flags
        return m_fd;

    case SYS_epoll_ctl:
        return sys_epoll_ctl((int)a0,
                            (int)a1,
                            (int)a2,
                            (struct epoll_event *)(uintptr_t)a3);

    case SYS_epoll_wait:
        return sys_epoll_wait((int)a0, (struct epoll_event *)(uintptr_t)a1, (int)a2, (int)a3); // epfd, events*, maxevents, timeout

    case SYS_preadv:
        // (you can stub it for now)
        log_error("SYSCALL", "SYS_preadv not implemented yet");
        return -ENOSYS;

    case SYS_pwritev:
        // pwritev (old) – you can implement or just route to pwritev2 with flags=0
        return sys_pwritev2(a0, a1, a2, a3, 0, 0);

    case SYS_preadv2:
        log_error("SYSCALL", "SYS_preadv2 not implemented yet");
        return -ENOSYS; // until you implement it

    case SYS_pwritev2:
        return sys_pwritev2(a0, a1, a2, a3, a4, a5);

    case SYS_membarrier:
        return sys_membarrier((int)a0, (int)a1);


    case SYS_test:
        log_info("SYSCALL", "TEST: a0=%llx a1=%llx a2=%llx a3=%llx a4=%llx a5=%llx",
                (unsigned long long)a0,
                (unsigned long long)a1,
                (unsigned long long)a2,
                (unsigned long long)a3,
                (unsigned long long)a4,
                (unsigned long long)a5);
        return 12345;
    }

    log_error("SYSCALL", "Unknown syscall %llu", (unsigned long long)nr);

    // log_error("SYSCALL", "Infinite while loop");
    // while(1)
    // {

    // }
    return (uint64_t)-1;
}

