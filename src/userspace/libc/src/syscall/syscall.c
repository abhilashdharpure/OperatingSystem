#include <stdint.h>
#include <syscall.h>

// ssize_t syscall_write(int fd, const void *buf, size_t len)
// {
//     return (ssize_t)syscall3(SYS_write, fd, (long)buf, (long)len);
// }

// __attribute__((noreturn))
// void syscall_exit(int code)
// {
//     asm volatile (
//         "syscall"
//         :
//         : "a"(SYS_exit),
//           "D"(code)
//         : "rcx", "r11", "memory"
//     );

//     __builtin_unreachable();
// }

long syscall0(long n)
{
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long syscall3(long n, long a, long b, long c)
{
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long syscall(long n, long a, long b, long c)
{
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
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