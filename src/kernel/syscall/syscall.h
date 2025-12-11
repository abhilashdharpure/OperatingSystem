#pragma once

#include <stdint.h>

/* syscall numbers (must match userspace) */
#define SYS_WRITE  1

static int copy_from_user(void *dst, const void *uaddr, uint32_t len);
static fd_t map_user_fd_to_vfs(int user_fd);
void i686_syscall_handler(Registers* regs);

