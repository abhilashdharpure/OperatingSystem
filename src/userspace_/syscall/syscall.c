#include <stdint.h>
#include "syscall.h"
// #include "debug.h"

struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

int sys_write(int fd, const char *buf, uint32_t len);

void syscall_dispatch(struct regs *r)
{
    switch (r->eax) {
        case SYS_WRITE:
            r->eax = sys_write(
                r->ebx,   // fd
                (const char *)r->ecx,
                r->edx    // len
            );
            break;

        default:
            // log_debug("SYSCALL", "Unknown syscall %d", r->eax);
            r->eax = -1;
    }
}
