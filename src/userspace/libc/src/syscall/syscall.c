#include <stdint.h>
#include <sys/syscall.h>
#include <stdio.h>

long syscall(long n, long a, long b, long c, long d)
{
    long ret;
    asm volatile (
        "mov %5, %%r10\n\t"   // use %5, not %4
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c), "r"(d)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// long syscall6(long n, long a, long b, long c, long d, long e, long f)
// {
//     long ret;
//     asm volatile (
//         "mov %5, %%r10\n\t"
//         "mov %6, %%r8\n\t"
//         "mov %7, %%r9\n\t"
//         "syscall"
//         : "=a"(ret)
//         : "a"(n), "D"(a), "S"(b), "d"(c), "r"(d), "r"(e), "r"(f)
//         : "rcx", "r11", "memory"
//     );
//     return ret;
// }

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
          "r"(r10),    // will be placed in r10
          "r"(r8),     // will be placed in r8
          "r"(r9)      // will be placed in r9
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