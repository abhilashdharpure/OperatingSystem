#pragma once
#include <stdint.h>
#include "hal/vfs.h"

struct eventfd_ctx {
    uint64_t counter;
    int flags;
};

extern struct file_operations eventfd_fops;