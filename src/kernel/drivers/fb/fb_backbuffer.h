#pragma once

#include "drivers/fb/fb.h"
#include <util/string.h>
// #include <stdlib.h>

void fb_backbuffer_init(void);
void fb_backbuffer_clear(uint32_t color);
void fb_backbuffer_put_pixel(int x, int y, uint32_t color);
void fb_backbuffer_flush(void);