#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int32_t ssize_t;

ssize_t write(int fd, const void *buf, size_t len);
void _exit(int code);
void exit(int code);
