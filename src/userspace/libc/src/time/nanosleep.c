// libc/time/nanosleep.c
#include <time.h>
#include <syscall.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    (void)rem; // not implemented yet

    long ret = syscall(SYS_nanosleep,
                       (long)req,
                       0,
                       0);
    return (int)ret;
}
