#include <stdint.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdarg.h>

long syscall(long nr, ...)
{
    va_list ap;
    va_start(ap, nr);

    long a0 = va_arg(ap, long);
    long a1 = va_arg(ap, long);
    long a2 = va_arg(ap, long);
    long a3 = va_arg(ap, long);
    long a4 = va_arg(ap, long);
    long a5 = va_arg(ap, long);

    va_end(ap);

    return syscall6(nr, a0, a1, a2, a3, a4, a5);
}

long syscall6(long nr,
              long a0, long a1, long a2,
              long a3, long a4, long a5)
{
    long ret;
    __asm__ volatile (
        "movq %5, %%r10\n\t"   /* a3 -> r10 */
        "movq %6, %%r8\n\t"    /* a4 -> r8  */
        "movq %7, %%r9\n\t"    /* a5 -> r9  */
        "syscall"
        : "=a"(ret)
        : "a"(nr),             /* rax = nr  */
          "D"(a0),             /* rdi = a0  */
          "S"(a1),             /* rsi = a1  */
          "d"(a2),             /* rdx = a2  */
          "r"(a3),             /* temp for r10 */
          "r"(a4),             /* temp for r8  */
          "r"(a5)              /* temp for r9  */
        : "rcx", "r11", "r10", "r8", "r9", "memory"
    );
    return ret;
}

// //// Syscall using intterupt 0x80.
// ssize_t syscall_write(int fd, const void *buf, size_t len)
// {    
//     ssize_t ret;

//     asm volatile(
//         "int $0x80"
//         : "=a"(ret)
//         : "a"(SYS_write), "b"(fd), "c"(buf), "d"(len)
//         : "memory"
//     );

//     return ret;
// }

// __attribute__((noreturn))
// void syscall_exit(int code)
// {
//     asm volatile(
//         "int $0x80"
//         :
//         : "a"(SYS_exit), "b"(code)
//         : "memory"
//     );
// }