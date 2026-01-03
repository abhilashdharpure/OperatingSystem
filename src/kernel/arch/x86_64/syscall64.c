// syscall64.c
#include "syscall64.h"
#include "syscall_numbers.h"
#include "msr.h"
#include "debug.h"

extern void x64_syscall_entry(void);

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084

void x64_SYSCALL_Initialize(void)
{
    uint64_t efer = rdmsr(IA32_EFER);
    efer |= (1ull << 0);              // SCE: enable SYSCALL/SYSRET
    wrmsr(IA32_EFER, efer);

    uint16_t kernel_cs = 0x08;        // your 64-bit code segment
    uint16_t user_cs   = 0x1B;        // TODO: define ring3 CS in GDT later

    // uint64_t star = 0;
    // star |= ((uint64_t)kernel_cs << 32);
    // star |= ((uint64_t)user_cs   << 48);

    // uint64_t star =
    //     ((uint64_t)0x08 << 32) |   // kernel CS
    //     ((uint64_t)0x10 << 48);    // user CS base (user CS - 16)

    uint64_t star =
        ((uint64_t)0x08 << 32) |   // kernel CS
        ((uint64_t)(0x23 - 0x10) << 48); // user CS base

    wrmsr(IA32_STAR, star);

    wrmsr(IA32_LSTAR, (uint64_t)&x64_syscall_entry);

    uint64_t fmask = (1ull << 9) | (1ull << 8);  // IF, TF
    wrmsr(IA32_FMASK, fmask);

    log_info("SYSCALL", "SYSCALL init: LSTAR=%p STAR=0x%llx EFER=0x%llx",
             (void*)&x64_syscall_entry,
             (unsigned long long)star,
             (unsigned long long)efer);
}

uint64_t syscall_dispatch(syscall_regs_t *r)
{
    uint64_t syscall_nr;
    __asm__ volatile ("mov %%rax, %0" : "=r"(syscall_nr));

    switch (syscall_nr) {
    case SYS_write:
        return sys_write(
            (int)r->rdi,
            (const void*)r->rsi,
            (size_t)r->rdx
        );

    case SYS_test:
        log_info("SYSCALL", "SYS_test from userspace!");
        return 123;

    case SYS_exit:
        sys_exit((int)r->rdi);
        __builtin_unreachable();
    }

    return (uint64_t)-1;
}


// uint64_t syscall_dispatch(syscall_regs_t *r)
// {
//     switch (r->rax) {
//     case SYS_write:
//         return sys_write(
//             (int)r->rdi,        // fd
//             (const void*)r->rsi,// buf
//             (size_t)r->rdx      // len
//         );

//     case SYS_test:
//         log_info("SYSCALL", "SYS_test from userspace!");
//         return 123;

//     case SYS_exit:
//         sys_exit((int)r->rdi);
//         __builtin_unreachable();
//     }

//     return -1;
// }
