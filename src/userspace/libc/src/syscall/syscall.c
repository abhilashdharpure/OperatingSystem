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

// long my_raw_syscall6(long n,
//                                    long a0, long a1, long a2,
//                                    long a3, long a4, long a5)
// {
//     long ret;
//     register long r10 __asm__("r10") = a3;
//     register long r8  __asm__("r8")  = a4;
//     register long r9  __asm__("r9")  = a5;

//     __asm__ volatile("syscall"
//         : "=a"(ret)
//         : "a"(n),
//           "D"(a0),
//           "S"(a1),
//           "d"(a2),
//           "r"(r10),
//           "r"(r8),
//           "r"(r9)
//         : "rcx", "r11", "r10", "r8", "r9", "memory");

//     return ret;
// }

// long k_syscall6(long nr,
//                 long a0, long a1, long a2,
//                 long a3, long a4, long a5)
// {
//     long ret;
//     __asm__ volatile (
//         "movq %5, %%r10\n\t"   // a3 -> r10
//         "movq %6, %%r8\n\t"    // a4 -> r8
//         "movq %7, %%r9\n\t"    // a5 -> r9
//         "syscall"
//         : "=a"(ret)
//         : "a"(nr),             // rax = nr
//           "D"(a0),             // rdi = a0
//           "S"(a1),             // rsi = a1
//           "d"(a2),             // rdx = a2
//           "r"(a3),             // temp for r10
//           "r"(a4),             // temp for r8
//           "r"(a5)              // temp for r9
//         : "rcx", "r11", "r10", "r8", "r9", "memory"
//     );
//     return ret;
// }

long my_raw_syscall6(long nr,
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




// long syscall6(long n, long a, long b, long c, long d, long e, long f)
// {
//     long ret;

//     register long r10 __asm__("r10") = d;
//     register long r8  __asm__("r8")  = e;
//     register long r9  __asm__("r9")  = f;

//     asm volatile (
//         "syscall"
//         : "=a"(ret)
//         : "a"(n),      // rax = syscall number
//           "D"(a),      // rdi = arg0
//           "S"(b),      // rsi = arg1
//           "d"(c),      // rdx = arg2
//           "r"(r10),    // GCC already placed r10 in register r10
//           "r"(r8),     // GCC already placed r8  in register r8
//           "r"(r9)      // GCC already placed r9  in register r9
//         : "rcx", "r11", "memory"
//     );

//     return ret;
// }








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