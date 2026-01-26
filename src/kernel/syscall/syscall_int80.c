#include "syscall_common.h"
#include "syscall_numbers.h"
#include <arch/x86_64/isr64.h>
#include "debug.h"

void x64_syscall_handler(ISRFrame64 *r)
{
    log_info("SYSCALL", "entry: rip=0x%llx rsp=0x%llx rax=%lld",
             (unsigned long long)r->cpu.rip,
             (unsigned long long)r->cpu.rsp,
             (long long)r->rax);

    uint64_t num = r->rax;
    uint64_t a0  = r->rbx;
    uint64_t a1  = r->rcx;
    uint64_t a2  = r->rdx;

    switch (num) {
    case SYS_write:
        r->rax = sys_write(a0, (const char*)a1, a2);
        break;

    case SYS_exit:
        sys_exit(a0);
        break;

    default:
        log_error("SYSCALL", "Unknown int80 syscall %llu", num);
        r->rax = (uint64_t)-1;
    }

    log_info("SYSCALL", "exit:  rip=0x%llx rsp=0x%llx rax=%lld",
             (unsigned long long)r->cpu.rip,
             (unsigned long long)r->cpu.rsp,
             (long long)r->rax);
}
