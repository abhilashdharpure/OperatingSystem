// bochs.c
#include "bochs.h"
#include "arch/x86_64/io.h"
#include "debug.h"

#define DISPI_INDEX 0x01CE
#define DISPI_DATA  0x01CF
enum { D_ID, D_XRES, D_YRES, D_BPP, D_ENABLE, D_BANK, D_VIRT_W, D_VIRT_H, D_XOFF, D_YOFF, D_VRAM64K };

static int g_present;
static uint32_t g_vram;

static void     dispi_w(uint16_t i, uint16_t v) { outw(DISPI_INDEX, i); outw(DISPI_DATA, v); }
static uint16_t dispi_r(uint16_t i)             { outw(DISPI_INDEX, i); return inw(DISPI_DATA); }

int bochs_init(void)
{
    uint16_t id = dispi_r(D_ID);
    g_present = id >= 0xB0C0 && id <= 0xB0C5;
    if (!g_present) { log_info("BOCHS", "no Bochs/QEMU VGA (id=0x%x)", id); return 0; }
    g_vram = (uint32_t)dispi_r(D_VRAM64K) * 65536u;
    if (!g_vram) g_vram = 16u << 20;                      /* older devices: assume 16 MB */
    log_info("BOCHS", "adapter id=0x%x vram=%u MB", id, g_vram >> 20);
    return 1;
}

int      bochs_present(void)     { return g_present; }
uint32_t bochs_vram_bytes(void)  { return g_vram; }
uint16_t bochs_virt_height(void) { return dispi_r(D_VIRT_H); }
void     bochs_set_y(uint16_t y) { dispi_w(D_YOFF, y); }

int bochs_set_mode(uint16_t w, uint16_t h, uint16_t virt_h)
{
    if (!g_present) return -1;
    dispi_w(D_ENABLE, 0);
    dispi_w(D_BPP, 32);
    dispi_w(D_XRES, w);
    dispi_w(D_YRES, h);
    dispi_w(D_VIRT_W, w);
    dispi_w(D_VIRT_H, virt_h);
    dispi_w(D_XOFF, 0);
    dispi_w(D_YOFF, 0);
    dispi_w(D_ENABLE, 0x01 | 0x40);                       /* enabled + linear framebuffer */
    if (dispi_r(D_XRES) != w || dispi_r(D_YRES) != h) {
        log_error("BOCHS", "mode %ux%u rejected", w, h);
        return -1;
    }
    return 0;
}