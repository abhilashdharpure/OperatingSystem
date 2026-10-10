#pragma once
#include <stdint.h>
#include "kernel_poll.h"

struct timespec
{
long tv_sec;
long tv_nsec;
};

static uint64_t ktime_monotonic_ms(void);
static int poll_do_scan(struct pollfd *fds, uint64_t nfds);