#include "fb_graphics.h"
#include "fb.h"   // your framebuffer header

static int gfx_width;
static int gfx_height;
static int gfx_pitch;
static uint32_t* gfx_buffer;

void gfx_init(void)
{
    gfx_width  = fb.width;
    gfx_height = fb.height;
    gfx_pitch  = fb.pitch / 4; // pitch in uint32_t
    gfx_buffer = (uint32_t*)fb.framebuffer;
}

// Draw a single pixel
void gfx_put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= gfx_width || y < 0 || y >= gfx_height)
        return;
    gfx_buffer[y * gfx_pitch + x] = color;
}

uint32_t gfx_get_pixel(int x, int y)
{
    if (x < 0 || x >= gfx_width || y < 0 || y >= gfx_height)
        return 0;
    return gfx_buffer[y * gfx_pitch + x];
}

// Horizontal line
void gfx_draw_hline(int x, int y, int width, uint32_t color)
{
    for (int i = 0; i < width; i++)
        gfx_put_pixel(x + i, y, color);
}

// Vertical line
void gfx_draw_vline(int x, int y, int height, uint32_t color)
{
    for (int i = 0; i < height; i++)
        gfx_put_pixel(x, y + i, color);
}

// Filled rectangle
void gfx_fill_rect(int x, int y, int width, int height, uint32_t color)
{
    for (int j = 0; j < height; j++)
        gfx_draw_hline(x, y + j, width, color);
}

// Clear screen
void gfx_clear(uint32_t color)
{
    for (int y = 0; y < gfx_height; y++)
        gfx_draw_hline(0, y, gfx_width, color);
}
