// libc/src/socket/socketpair.c
#include <sys/socket.h>
#include <sys/syscall.h>
#include <stdio.h>

extern long syscall6(long nr,
                     long a0, long a1, long a2,
                     long a3, long a4, long a5);

int socketpair(int domain, int type, int protocol, int sv[2])
{
    printf("socketpair(): sv=%p\n", (void*)sv);
    long ret = syscall6(SYS_socketpair,
                       domain, type, protocol,
                       (long)(uintptr_t)sv,
                       0L, 0L);
    printf("socketpair(): after syscall, sv=%p\n", (void*)sv);
    return (int)ret;
}
