// bochs.h
#pragma once
#include <stdint.h>
int      bochs_init(void);                 // probes the adapter; returns 1 if present
int      bochs_present(void);
uint32_t bochs_vram_bytes(void);
int      bochs_set_mode(uint16_t w, uint16_t h, uint16_t virt_h);   // 0 = ok
uint16_t bochs_virt_height(void);
void     bochs_set_y(uint16_t y);