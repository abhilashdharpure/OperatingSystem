// userspace/libc/src/sys/execve/execve.c

#include <stdint.h>
#include <sys/syscall.h>

long execve(const char *path, char *const argv[], char *const envp[])
{
    register long rax __asm__("rax") = SYS_execve;
    register long rdi __asm__("rdi") = (long)path;
    register long rsi __asm__("rsi") = (long)argv;
    register long rdx __asm__("rdx") = (long)envp;

    __asm__ volatile (
        "syscall"
        : "+r"(rax)
        : "r"(rdi), "r"(rsi), "r"(rdx)
        : "rcx", "r11", "memory"
    );

    return rax;
}
