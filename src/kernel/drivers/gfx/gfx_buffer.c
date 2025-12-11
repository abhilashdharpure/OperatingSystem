#include "gfx_buffer.h"
#include <kmalloc.h>

gfx_buffer_t* gfx_buffer_create(int w, int h)
{
    gfx_buffer_t* buf = kmalloc(sizeof(gfx_buffer_t));
    buf->width = w;
    buf->height = h;
    buf->pitch = w; // pixel pitch
    buf->pixels = kmalloc(w * h * sizeof(uint32_t));
    return buf;
}

void gfx_buffer_destroy(gfx_buffer_t* buf)
{
    kfree(buf->pixels);
    kfree(buf);
}
