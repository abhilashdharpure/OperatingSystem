#include <stdint.h>
#include <stddef.h>
#include "debug.h"
#include <arch/i686/isr.h>
#include "syscall.h"
#include "hal/vfs.h"     /* for VFS_Write and VFS_FD_* constants */
#include "hal/process.h"     /* for current_process if you want pid in logging */

/* If you already have a safe copy_from_user helper, use it.
   Otherwise the simple version below tries to be cautious: it will
   check that the virtual address lies in user-space range if you have a helper.
   Replace `is_user_vaddr()` with your kernel's equivalent. */

extern int is_user_vaddr(uint32_t vaddr); /* optional - replace or remove if not present */

static void syscall_write(Registers *regs);

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

#define STDOUT 1
#define STDERR 2

/* Map user fds to kernel VFS fds used by your stdio implementation */
static fd_t map_user_fd_to_vfs(int user_fd)
{
    switch (user_fd) {
        case STDOUT: return VFS_FD_STDOUT;
        case STDERR: return VFS_FD_STDERR;
        default:     return (fd_t)-1;
    }
}

/* Replace kernel_putchar with your actual routine if needed (e.g. framebuffer/serial writer).
   Here we rely on VFS_Write so nothing else is needed. */
void i686_syscall_handler(Registers* regs)
{
    // log_debug("SYSCALL", "eax=%u ebx=%u ecx=%p edx=%u",
    //           regs->eax, regs->ebx, regs->ecx, regs->edx);

    log_info("SYSCALL",
             "### syscall: eax=%u ebx=%u ecx=%p edx=%u cs=%x eip=%x",
             regs->eax, regs->ebx, regs->ecx, regs->edx,
             regs->cs, regs->eip);

    if (regs->eax == 1) {  // SYS_WRITE

        switch (regs->eax) {
            case SYS_WRITE:
                syscall_write(regs);
                break;

            default:
                log_error("SYSCALL", "Unknown syscall %u", regs->eax);
                break;
        }


        // const char *buf = (const char *)regs->ecx;
        // uint32_t len   = regs->edx;

        // for (uint32_t i = 0; i < len; ++i)
        //     kputc(buf[i]);  // direct VGA

        // regs->eax = (int)len;
        return;
    }

    log_error("SYSCALL", "Unknown syscall %u", regs->eax);


}

static void syscall_write(Registers *regs)
{
    int fd = regs->ebx;
    const char *buf = (const char *)regs->ecx;
    uint32_t len = regs->edx;

    log_debug("SYSCALL", "SYS_WRITE fd=%d buf=%p len=%u", fd, buf, len);

    /* VERY IMPORTANT:
       User memory must be mapped in kernel page tables.
       If you are using shared user/kernel address space, this works.
    */

    int ret = VFS_Write(fd, buf, len);
    regs->eax = ret;
}
// void i686_syscall_handler(Registers* regs)
// {
//     uint32_t num = regs->eax;

//     switch (num)
//     {
//         case SYS_WRITE:
//         {
//             int user_fd = (int)regs->ebx;
//             const void *user_buf = (const void *)regs->ecx;
//             uint32_t len = regs->edx;

//             log_debug("SYSCALL", "write(fd=%d, buf=%p, len=%u)", user_fd, user_buf, len);

//             fd_t vfs_fd = map_user_fd_to_vfs(user_fd);
//             if (vfs_fd == (fd_t)-1) {
//                 regs->eax = (uint32_t)-1;
//                 return;
//             }

//             /* Cap per-chunk copy to avoid large stack arrays. We'll copy in chunks. */
//             const uint32_t CHUNK = 1024;
//             uint32_t remaining = len;
//             uint32_t offset = 0;
//             char tmp[CHUNK];

//             while (remaining > 0) {
//                 uint32_t to_copy = (remaining > CHUNK) ? CHUNK : remaining;

//                 if (copy_from_user(tmp, (const uint8_t*)user_buf + offset, to_copy) != 0) {
//                     log_error("SYSCALL", "copy_from_user failed for buf=%p len=%u", user_buf, to_copy);
//                     regs->eax = (uint32_t)-1;
//                     return;
//                 }

//                 /* Write the chunk to the VFS. VFS_Write should synchronously deliver to console. */
//                 int written = VFS_Write(vfs_fd, tmp, to_copy);
//                 if (written < 0) {
//                     log_error("SYSCALL", "VFS_Write failed fd=%d", vfs_fd);
//                     regs->eax = (uint32_t)-1;
//                     return;
//                 }

//                 remaining -= to_copy;
//                 offset += to_copy;
//             }

//             /* Return total written bytes */
//             regs->eax = len;
//             break;
//         }

//         default:
//             log_error("SYSCALL", "Unknown syscall %u", num);
//             regs->eax = (uint32_t)-1;
//             break;
//     }
// }
