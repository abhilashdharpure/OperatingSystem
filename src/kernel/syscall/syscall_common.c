#include "syscall_common.h"
#include "debug.h"

ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len)
{
    (void)fd;
    for (uint64_t i = 0; i < len; ++i)
    {
        serial_putc(buf[i]);
    }

    return len;
}

__attribute__((noreturn))
void sys_exit(uint64_t code)
{
    log_info("SYSCALL", "userspace exited with code %llu",
             (unsigned long long)code);

    // Instead of halting, spin with a heartbeat
    for (;;) {
        log_info("SYSCALL", "spinning after exit...");
        for (volatile uint64_t i = 0; i < 100000000; ++i) { }
    }
}

