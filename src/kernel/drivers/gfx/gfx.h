#pragma once
#include <stdint.h>

typedef struct {
    int width;
    int height;
    int pitch;
    int bpp;

    // Your backbuffer
    uint32_t* backbuffer;

    // API
    void (*clear)(uint32_t color);
    void (*put_pixel)(int x, int y, uint32_t color);
    void (*present)(void);
} gfx_device_t;

extern gfx_device_t gfx;
void gfx_init(void);
