#pragma once

#include "errno.h"
#include "kmalloc.h"   // kmalloc/kfree
#include <string.h>
#include <stdint.h>


long sys_epoll_create1(uint64_t flags);
long sys_eventfd2(unsigned int initval, int flags);