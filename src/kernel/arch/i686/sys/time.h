#pragma once

#include <stdint.h>

struct timeval {
    int64_t tv_sec;   // seconds
    int64_t tv_usec;  // microseconds
};