#pragma once
#include <stdint.h>
#include <stdint.h>

typedef int64_t ssize_t;

ssize_t sys_write(uint64_t fd, const char *buf, uint64_t len);

__attribute__((noreturn))
void sys_exit(uint64_t code);
