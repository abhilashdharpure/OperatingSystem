# pragma once


#include <stdint.h>
#include <boot/bootparams.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uintptr_t framebuffer;
} fb_device_t;

static fb_device_t fb_dev;

void fb_init(VbeModeInfo* info);
void fb_put_pixel(int x, int y, uint32_t color);
void fb_clear(uint32_t color);
void fb_draw_rect(int x, int y, int w, int h, uint32_t color);
