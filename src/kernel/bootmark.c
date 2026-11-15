#include <stdint.h>
#include "drivers/fb/fb.h"
#include "debug.h"
#include <stddef.h>

static inline int fb_ok(void) { return fb_dev.framebuffer && fb_dev.bpp == 32; }

void boot_mark(int slot, uint32_t rgb)            /* a 24x24 square in the top-left corner, slot 0,1,2... */
{
    if (!fb_ok()) return;
    for (int y = 0; y < 24; y++) {
        uint32_t *row = (uint32_t *)((uint8_t *)(uintptr_t)fb_dev.framebuffer + (size_t)y * fb_dev.pitch);
        for (int x = 0; x < 24; x++) row[slot * 28 + x] = rgb;
    }
}

void boot_fill(uint32_t rgb)                      /* whole screen: call from panic() with red */
{
    if (!fb_ok()) return;
    for (uint32_t y = 0; y < fb_dev.height; y++) {
        uint32_t *row = (uint32_t *)((uint8_t *)(uintptr_t)fb_dev.framebuffer + (size_t)y * fb_dev.pitch);
        for (uint32_t x = 0; x < fb_dev.width; x++) row[x] = rgb;
    }
}
void boot_bits(int row, uint8_t v, uint32_t on)       /* row 0..4, below the stage marks */
{
    if (!fb_ok()) return;
    for (int bit = 0; bit < 8; bit++) {
        uint32_t c = (v & (0x80 >> bit)) ? on : 0x00303030;
        for (int y = 0; y < 10; y++) {
            uint32_t *p = (uint32_t *)((uint8_t *)(uintptr_t)fb_dev.framebuffer
                                       + (size_t)(32 + row * 14 + y) * fb_dev.pitch);
            for (int x = 0; x < 10; x++) p[bit * 14 + x] = c;
        }
    }
}