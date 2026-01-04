#pragma once
#include <stdint.h>

struct kstat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    uint64_t st_size;
};
