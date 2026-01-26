#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    long ret = syscall(SYS_clock_gettime,
                       (long)clk_id,
                       (long)tp,
                       0,
                       0);
    if (ret < 0) {
        // set errno in libc if you want
        return -1;
    }
    return 0;
}
