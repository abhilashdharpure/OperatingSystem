// libc/src/socket/socketpair.c
#include <sys/socket.h>
#include <syscall.h>
#include <stdio.h>

// Add this:
#undef syscall
extern long syscall(long n, long a, long b, long c, long d);

int socketpair(int domain, int type, int protocol, int sv[2])
{
    printf("sv ptr in userspace: %p\n", (void*)sv);

    return syscall(SYS_socketpair,
                   (long)domain,
                   (long)type,
                   (long)protocol,
                   (long)sv);
}
