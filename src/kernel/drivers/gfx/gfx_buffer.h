#pragma once

#include <stdint.h>

typedef struct scons {
    int width;
    int height;
    int pitch;
    uint32_t* pixels;  // software buffer
} gfx_buffer_t;
