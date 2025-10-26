#include "fb_backbuffer.h"
#include "memory.h"
#include "kmalloc.h"

static uint32_t* backbuffer = NULL;

void fb_backbuffer_init(void)
{
    backbuffer = kmalloc(fb_dev.width * fb_dev.height * sizeof(uint32_t));
    memset(backbuffer, 0, fb_dev.width * fb_dev.height * sizeof(uint32_t));
}

void fb_backbuffer_clear(uint32_t color)
{
    for (uint32_t i = 0; i < fb_dev.width * fb_dev.height; i++)
        backbuffer[i] = color;
}

void fb_backbuffer_put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || y < 0 || x >= fb_dev.width || y >= fb_dev.height)
        return;
    backbuffer[y * fb_dev.width + x] = color;
}

void fb_backbuffer_flush(void)
{
    memcpy((void*)fb_dev.framebuffer, backbuffer,
           fb_dev.width * fb_dev.height * sizeof(uint32_t));
}