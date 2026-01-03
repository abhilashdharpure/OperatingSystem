#pragma once

#include <stddef.h>
#include <stdint.h>
#include <types.h>

ssize_t write(int fd, const void *buf, size_t len);
void _exit(int code);
void exit(int code);
