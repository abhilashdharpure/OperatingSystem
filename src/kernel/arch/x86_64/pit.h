#pragma once

#include <stdint.h>


void pit_init(uint32_t frequency);
uint64_t get_system_time_us(void);
uint64_t pit_get_frequency();