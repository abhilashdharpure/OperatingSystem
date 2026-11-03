#pragma once
#include <stdint.h>

typedef int clockid_t;

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

struct timespec {
    long tv_sec;
    long tv_nsec;
};

uint64_t get_system_time(void);
uint64_t get_system_time_ms(void);

