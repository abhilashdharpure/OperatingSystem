#include "gfx.h"
#include "../fb/fb.h"
#include "../fb/fb_backbuffer.h"
#include "debug.h"

gfx_device_t gfx;

void gfx_init(void)
{
    gfx.width  = fb_dev.width;
    gfx.height = fb_dev.height;
    gfx.pitch  = fb_dev.pitch;
    gfx.bpp    = fb_dev.bpp;

    fb_backbuffer_init();
    gfx.backbuffer = fb_backbuffer_get_buffer();  // add a getter

    gfx.clear     = fb_backbuffer_clear;
    gfx.put_pixel = fb_backbuffer_put_pixel;
    gfx.present   = fb_backbuffer_flush;

    log_info("GFX", "Graphics subsystem initialized");
}
