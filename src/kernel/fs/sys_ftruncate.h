#pragma once

#include <stdint.h>
#include <stddef.h>
#include "memfd.h"

int sys_ftruncate(int fd, off_t length);

int memfd_truncate(memfd_t *m, size_t new_size);
