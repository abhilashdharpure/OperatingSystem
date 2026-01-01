#include <stdint.h>
#include <stddef.h>
#include "debug.h"
#include <arch/x86_64/isr64.h>
#include "syscall.h"
#include "hal/vfs.h"     /* for VFS_Write and VFS_FD_* constants */
#include "hal/process.h"     /* for current_process if you want pid in logging */

#define STDOUT 1
#define STDERR 2

/* If you already have a safe copy_from_user helper, use it.
   Otherwise the simple version below tries to be cautious: it will
   check that the virtual address lies in user-space range if you have a helper.
   Replace `is_user_vaddr()` with your kernel's equivalent. */

extern int is_user_vaddr(uint32_t vaddr); /* optional - replace or remove if not present */

static void syscall_write(ISRFrame64 *regs);

/* copy_from_user: copies up-to `len` bytes from user address `uaddr` into kernel buffer dst.
   Returns 0 on success, -1 on invalid user address. */
static int copy_from_user(void *dst, const void *uaddr, uint32_t len)
{
    if (!dst || !uaddr) return -1;

#ifdef HAS_IS_USER_VADDR /* if your kernel defines this, define macro when compiling */
    uint32_t start = (uint32_t)uaddr;
    for (uint32_t i = 0; i < len; ++i) {
        if (!is_user_vaddr(start + i)) return -1;
    }
    /* safe to memcpy */
    memcpy(dst, uaddr, len);
    return 0;
#else
    /* Fallback naive approach:
       - Attempt to copy in small chunks to reduce risk of big allocations.
       - If a page fault occurs here, ensure your page-fault handler logs it and kills the process.
       Ideally replace this with your actual user-range validation function. */
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)uaddr;
    for (uint32_t i = 0; i < len; ++i) {
        /* best-effort: attempt the copy, but it may fault if user addr invalid.
           If your kernel traps page faults and recovers/kills the process that's okay. */
        d[i] = s[i];
    }
    return 0;
#endif
}

/* Map user fds to kernel VFS fds used by your stdio implementation */
static fd_t map_user_fd_to_vfs(int user_fd)
{
    switch (user_fd) {
        case STDOUT: return VFS_FD_STDOUT;
        case STDERR: return VFS_FD_STDERR;
        default:     return (fd_t)-1;
    }
}

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t len)
{
    (void)fd; // ignore fd for now, always write to serial

    for (uint64_t i = 0; i < len; ++i) {
        serial_putc(buf[i]);
    }

    return len;
}

void sys_exit(uint64_t status)
{
    log_info("SYSCALL", "Process exited with code %llu",
             (unsigned long long)status);

    // TODO: real process teardown; for now hang
    for (;;) {
        __asm__ volatile ("hlt");
    }
}


void x64_syscall_handler(ISRFrame64* r)
{
    // log_info("SYSCALL", "x64_syscall_handler entered");

    uint64_t num = r->rax;
    uint64_t a0  = r->rbx;
    uint64_t a1  = r->rcx;
    uint64_t a2  = r->rdx;

    switch (num) {
    case 1: // write(fd, buf, len)
        r->rax = sys_write(a0, (const char*)a1, a2);
        break;

    case 2: // exit(status)
        sys_exit(a0);
        break; // not reached

    default:
        log_error("SYSCALL", "Unknown syscall %llu", num);
        r->rax = (uint64_t)-1;
        break;
    }
}

static void syscall_write(ISRFrame64 *regs)
{
    log_info("SYSCALL", "TODO Not implement syscall_write...");

    // int fd = regs->ebx;
    // const char *buf = (const char *)regs->ecx;
    // uint32_t len = regs->edx;

    // log_debug("SYSCALL", "SYS_WRITE fd=%d buf=%p len=%u", fd, buf, len);

    // /* VERY IMPORTANT:
    //    User memory must be mapped in kernel page tables.
    //    If you are using shared user/kernel address space, this works.
    // */

    // int ret = VFS_Write(fd, buf, len);
    // regs->eax = ret;
}