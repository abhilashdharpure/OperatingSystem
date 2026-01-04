#include <syscall.h>


void klog(const char *msg)
{
    syscall(SYS_klog, (long)msg, 0, 0, 0);
}
