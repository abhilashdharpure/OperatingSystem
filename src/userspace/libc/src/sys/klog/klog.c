#include "syscall.h"

void klog(const char *msg)
{
    syscall6(SYS_klog, (long)msg, 0, 0, 0, 0, 0);
}
