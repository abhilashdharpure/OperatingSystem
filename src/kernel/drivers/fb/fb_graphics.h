#pragma once
#include <stdint.h>
#include "color.h"

// Initialize graphics abstraction
void gfx_init(void);

// Basic pixel operations
void gfx_put_pixel(int x, int y, uint32_t color);
uint32_t gfx_get_pixel(int x, int y);

// Rectangle operations
void gfx_fill_rect(int x, int y, int width, int height, uint32_t color);
void gfx_clear(uint32_t color);

// Line operations
void gfx_draw_hline(int x, int y, int width, uint32_t color);
void gfx_draw_vline(int x, int y, int height, uint32_t color);