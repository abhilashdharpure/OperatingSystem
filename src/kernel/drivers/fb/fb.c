#include "fb.h"
#include "stdio.h"

static VbeModeInfo fb;

#define COLOR(r,g,b) ((b) | (g << 8) | (r << 16))


void fb_init(VbeModeInfo* info)
{
    fb = *info;


    printf("fb_init, width = %d, height = %d, address = %lu .\n", fb.width, fb.height, fb.framebuffer);


    uint32_t* fb = (uint32_t*)(info->framebuffer);

    int w = info->width;
    int h = info->height;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            fb[y * info->pitch / 4 + x] = COLOR(x, y, x+y);
            // fb_put_pixel(x, y, COLOR(x, y, x+y));
        }
    }
}

void fb_put_pixel(int x, int y, uint32_t color)
{
    //printf("fb_put_pixel, x = %d, y = %d, color = %lu .\n", x, y, color);

    uint32_t* fb_mem = (uint32_t*)fb.framebuffer;
    fb_mem[y * (fb.pitch / 4) + x] = color;
}

void fb_clear(uint32_t color)
{
    for (int y = 0; y < fb.height; y++)
        for (int x = 0; x < fb.width; x++)
            fb_put_pixel(x, y, color);
}