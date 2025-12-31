#pragma once

#include <stdint.h>

static inline void *phys_to_virt_bootstrap(uint64_t pa);

void paging_init_long_mode_globals(void);