#include "display.h"
#include "time/time.h"
#include "stdint.h"
#include "stdio.h"
#include "debug.h"
#include "hal/file.h"
#include <stdio.h>
#include "stddef.h"
#include <arch/x86_64/irq.h>

fb_device_t fb_dev;
int g_edid_valid = 0;
uint8_t g_edid[128];

static int text_read(struct file *f, void *buf, size_t size, const char *txt, size_t len)
{
    if (f->position >= len) return 0;
    size_t n = len - f->position;
    if (n > size) n = size;
    memcpy(buf, txt + f->position, n);
    f->position += n;
    return (int)n;
}

static int display_read(struct file *f, void *buf, size_t size)
{
    char t[192];
    ksnprintf(t, sizeof t, "width=%u\nheight=%u\npitch=%u\nbpp=%u\nrefresh_mhz=%u\nname=%s\n",
              fb_dev.width, fb_dev.height, fb_dev.pitch, fb_dev.bpp, 0u, "Boot framebuffer");
    return text_read(f, buf, size, t, strlen(t));
}

extern uint8_t g_edid[128];
extern int     g_edid_valid;                  // set by the boot code, 0 if none

static int edid_read(struct file *f, void *buf, size_t size)
{
    if (!g_edid_valid) return 0;
    return text_read(f, buf, size, (const char *)g_edid, 128);
}

static struct file_operations display_fops = { .read = display_read };
static struct file_operations edid_fops    = { .read = edid_read };

void display_devices_init(void)
{
    VFS_RegisterDevice("/dev/display0", &display_fops, NULL);
    VFS_RegisterDevice("/dev/edid0",    &edid_fops,    NULL);
}