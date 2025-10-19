# pragma once


#include <stdint.h>
#include <boot/bootparams.h>


void fb_init(VbeModeInfo* info);
void fb_put_pixel(int x, int y, uint32_t color);
void fb_clear(uint32_t color);
void fb_draw_rect(int x, int y, int w, int h, uint32_t color);
