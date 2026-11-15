#pragma once

#include <stdint.h>
// Simple key buffer
#define KEY_BUFFER_SIZE 32
#define SYN_REPORT 0

void ps2_enable_mouse();
void ps2_init();
void ps2_poll(void);
void kp_push(uint16_t type, uint16_t code, int32_t value);
