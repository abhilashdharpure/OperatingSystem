// syscall64.h
#pragma once
#include <stdint.h>

void x64_SYSCALL_Initialize(void);
uint64_t syscall_dispatch(uint64_t num,
                          uint64_t arg0,
                          uint64_t arg1,
                          uint64_t arg2,
                          uint64_t arg3,
                          uint64_t arg4);
