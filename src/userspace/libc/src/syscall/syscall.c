#include <stdint.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdarg.h>

long syscall(long n, ...)
{
    va_list ap;
    long a[6] = {0,0,0,0,0,0};

    va_start(ap, n);
    for (int i = 0; i < 6; ++i)
        a[i] = va_arg(ap, long);
    va_end(ap);

    return syscall6(n, a[0], a[1], a[2], a[3], a[4], a[5]);
}


long syscall6(long n, long a, long b, long c, long d, long e, long f)
{
    long ret;

    register long r10 __asm__("r10") = d;
    register long r8  __asm__("r8")  = e;
    register long r9  __asm__("r9")  = f;

    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n),      // rax = syscall number
          "D"(a),      // rdi = arg0
          "S"(b),      // rsi = arg1
          "d"(c),      // rdx = arg2
          "r"(r10),    // GCC already placed r10 in register r10
          "r"(r8),     // GCC already placed r8  in register r8
          "r"(r9)      // GCC already placed r9  in register r9
        : "rcx", "r11", "memory"
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