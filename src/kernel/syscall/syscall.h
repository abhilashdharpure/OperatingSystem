#pragma once

#include <stdint.h>
#include <arch/x86_64/isr64.h>


/* syscall numbers (must match userspace) */
#define SYS_WRITE  1

uint64_t sys_write(uint64_t fd, const char *buf, uint64_t len);
void sys_exit(uint64_t status) __attribute__((noreturn));

static int copy_from_user(void *dst, const void *uaddr, uint32_t len);
static fd_t map_user_fd_to_vfs(int user_fd);
// void i686_syscall_handler(Registers* regs);
void x64_syscall_handler(ISRFrame64* regs);
