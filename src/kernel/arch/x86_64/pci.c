#include <stdint.h>
#include "arch/x86_64/io.h"
#include "debug.h"
#include "ps2.h"
#include "arch/x86_64/events/input_event.h"

#ifndef inl
static inline void outl(uint16_t p, uint32_t v) { __asm__ volatile("outl %0,%1" :: "a"(v), "Nd"(p)); }
static inline uint32_t inl(uint16_t p) { uint32_t v; __asm__ volatile("inl %1,%0" : "=a"(v) : "Nd"(p)); return v; }
#endif

typedef struct { uint8_t bus, dev, fn; uint16_t vendor, device; uint8_t cls, sub, prog; } pci_dev_t;

uint32_t pci_read32(uint8_t b, uint8_t d, uint8_t f, uint8_t off)
{
    outl(0xCF8, 0x80000000u | ((uint32_t)b << 16) | ((uint32_t)d << 11) | ((uint32_t)f << 8) | (off & 0xFC));
    return inl(0xCFC);
}
void pci_write32(uint8_t b, uint8_t d, uint8_t f, uint8_t off, uint32_t v)
{
    outl(0xCF8, 0x80000000u | ((uint32_t)b << 16) | ((uint32_t)d << 11) | ((uint32_t)f << 8) | (off & 0xFC));
    outl(0xCFC, v);
}

static pci_dev_t g_pci[64];
static int g_npci;

void pci_scan(void)
{
    g_npci = 0;
    for (int b = 0; b < 256; b++)
        for (int d = 0; d < 32; d++) {
            if ((pci_read32(b, d, 0, 0) & 0xFFFF) == 0xFFFF) continue;
            int multi = (pci_read32(b, d, 0, 0x0C) >> 16) & 0x80;
            for (int f = 0; f < (multi ? 8 : 1); f++) {
                uint32_t id = pci_read32(b, d, f, 0);
                if ((id & 0xFFFF) == 0xFFFF) continue;
                uint32_t cl = pci_read32(b, d, f, 8);
                pci_dev_t p = { b, d, f, id & 0xFFFF, id >> 16, cl >> 24, (cl >> 16) & 0xFF, (cl >> 8) & 0xFF };
                if (g_npci < 64) g_pci[g_npci++] = p;
                log_info("PCI", "%x:%x.%x  %x:%x  class %x.%x.%x",
                         b, d, f, p.vendor, p.device, p.cls, p.sub, p.prog);
            }
        }
}

int pci_find(uint16_t vendor, uint16_t device, pci_dev_t *out)
{
    for (int i = 0; i < g_npci; i++)
    {
        // log_info("PCI", "pci_find g_pci[i].vendor = %d, vendor = %d, g_pci[i].device = %d, device = %d", g_pci[i].vendor, vendor, g_pci[i].device, device);

        if (g_pci[i].vendor == vendor && g_pci[i].device == device)
        {
            *out = g_pci[i];
            return 0;
        }
    }

    log_info("PCI", "pci_find returning -1");
    return -1;
}

int pci_find_class(uint8_t cls, uint8_t sub, uint8_t prog, pci_dev_t *out)
{
    for (int i = 0; i < g_npci; i++)
        if (g_pci[i].cls == cls && g_pci[i].sub == sub && g_pci[i].prog == prog) { *out = g_pci[i]; return 0; }
    return -1;
}

void mouse_inject(int dx, int dy, uint8_t buttons)        /* USB HID boot reports: bit0 left, bit1 right, bit2 middle; +dy is down */
{
    static uint8_t prev;
    uint8_t ch = prev ^ buttons;
    if (ch & 1) kp_push(EV_KEY, BTN_LEFT,   (buttons & 1) ? KEY_PRESSED : KEY_RELEASED);
    if (ch & 2) kp_push(EV_KEY, BTN_RIGHT,  (buttons & 2) ? KEY_PRESSED : KEY_RELEASED);
    if (ch & 4) kp_push(EV_KEY, BTN_MIDDLE, (buttons & 4) ? KEY_PRESSED : KEY_RELEASED);
    if (dx) kp_push(EV_REL, REL_X, dx);
    if (dy) kp_push(EV_REL, REL_Y, dy);
    kp_push(EV_SYN, SYN_REPORT, 0);
    prev = buttons;
}