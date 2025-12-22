// syscall64.c
#include "syscall64.h"
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

    uint64_t star = 0;
    star |= ((uint64_t)kernel_cs << 32);
    star |= ((uint64_t)user_cs   << 48);
    wrmsr(IA32_STAR, star);

    wrmsr(IA32_LSTAR, (uint64_t)&x64_syscall_entry);

    uint64_t fmask = (1ull << 9) | (1ull << 8);  // IF, TF
    wrmsr(IA32_FMASK, fmask);

    log_info("SYSCALL", "SYSCALL init: LSTAR=%p STAR=0x%llx EFER=0x%llx",
             (void*)&x64_syscall_entry,
             (unsigned long long)star,
             (unsigned long long)efer);
}

uint64_t syscall_dispatch(uint64_t num,
                          uint64_t arg0,
                          uint64_t arg1,
                          uint64_t arg2,
                          uint64_t arg3,
                          uint64_t arg4)
{
    log_debug("SYSCALL", "syscall %llu(%llu,%llu,%llu,%llu,%llu)",
              (unsigned long long)num,
              (unsigned long long)arg0,
              (unsigned long long)arg1,
              (unsigned long long)arg2,
              (unsigned long long)arg3,
              (unsigned long long)arg4);

    switch (num) {
        case 0:
            log_info("SYSCALL", "syscall 0 invoked");
            return 0;
        default:
            log_warning("SYSCALL", "unknown syscall %llu", (unsigned long long)num);
            return (uint64_t)-1;
    }
}
