#include "syscall64.h"
#include "syscall_common.h"
#include "syscall_numbers.h"
#include "debug.h"
#include "arch/x86_64/msr.h"

extern void x64_syscall_entry(void);

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084

void x64_SYSCALL_Initialize(void)
{
    uint64_t efer = rdmsr(IA32_EFER);
    efer |= (1ull << 0);              // SCE
    wrmsr(IA32_EFER, efer);

    uint16_t kernel_cs = 0x08;
    uint16_t user_cs   = 0x1B;

    uint64_t star =
        ((uint64_t)(user_cs - 0x10) << 48) |   // user base for SYSRET (we won't use it, but it's fine)
        ((uint64_t)kernel_cs        << 32);    // kernel CS for SYSCALL

    wrmsr(IA32_STAR,  star);
    wrmsr(IA32_LSTAR, (uint64_t)&x64_syscall_entry);

    uint64_t fmask = (1ull << 9) | (1ull << 8);  // IF, TF
    wrmsr(IA32_FMASK, fmask);
}

uint64_t sys_mmap_simple(uint64_t length,
                         uint64_t prot,
                         uint64_t flags)
{
    // addr = 0, fd = -1, offset = 0
    return sys_mmap(0, length, prot, flags, (uint64_t)-1, 0);
}


uint64_t syscall_dispatch(uint64_t nr,
                          uint64_t a0,
                          uint64_t a1,
                          uint64_t a2,
                          uint64_t a3,
                          uint64_t a4,
                          uint64_t a5)
{
    // log_info("SYSCALL", "syscall_dispatch: nr=%llu a0=%llx a1=%llx a2=%llx",
    //          (unsigned long long)nr,
    //          (unsigned long long)a0,
    //          (unsigned long long)a1,
    //          (unsigned long long)a2);

    switch (nr) {
    case SYS_write:
        return sys_write(a0, (const char *)a1, a2);

    case SYS_exit:
        sys_exit(a0);
        __builtin_unreachable();

    case SYS_open:
        return sys_open((const char *)a0, a1, a2);

    case SYS_read:
        return sys_read(a0, (void *)a1, a2);

    case SYS_close:
        return sys_close(a0);

    case SYS_mmap:
        return sys_mmap_simple(a0, a1, a2);

    case SYS_munmap:
        return sys_munmap(a0, a1);

    case SYS_mprotect:
        return sys_mprotect(a0, a1, a2);

    case SYS_brk:
        return sys_brk(a0);
    
    case SYS_poll:
        return sys_poll(a0, a1, a2);


    case SYS_test:
        return 123;
    }

    log_error("SYSCALL", "Unknown syscall %llu", (unsigned long long)nr);
    return (uint64_t)-1;
}