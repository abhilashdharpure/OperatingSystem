#include <stdint.h>
#include <sys/syscall.h> 

// Your existing syscall6
long syscall6(long n, long a, long b, long c, long d, long e, long f);

long __syscall(long n, long a, long b, long c, long d, long e, long f)
{
    return syscall6(n, a, b, c, d, e, f);
}

// Musl expects this too. For now, just alias it.
long __syscall_cp(long n, long a, long b, long c, long d, long e, long f)
{
    return syscall6(n, a, b, c, d, e, f);
}
