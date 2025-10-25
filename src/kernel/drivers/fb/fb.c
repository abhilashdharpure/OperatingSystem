#include "fb.h"
#include "stdio.h"
#include "hal/vfs.h"
#include "debug.h"



VbeModeInfo fb;

static int fb_open(struct file *f)
{
    // nothing special
    return 0;
}

static int fb_read(struct file *f, void *buf, size_t size)
{
    fb_device_t *dev = f->private_data;
    memcpy(buf, (void*)dev->framebuffer, size);
    return size;
}

static int fb_write(struct file *f, const void *buf, size_t size)
{
    fb_device_t *dev = f->private_data;
    memcpy((void*)dev->framebuffer, buf, size);
    return size;
}

static int fb_ioctl(struct file *f, int cmd, void *arg)
{
    fb_device_t *dev = f->private_data;
    switch (cmd)
    {
    case 0x4602: // FBIOGET_VSCREENINFO (like Linux)
        memcpy(arg, dev, sizeof(fb_device_t));
        return 0;
    default:
        return -1;
    }
}

struct file_operations fb_fops = {
    .open = fb_open,
    .read = fb_read,
    .write = fb_write,
    .ioctl = fb_ioctl,
};


void fb_init(VbeModeInfo* info)
{
    // // Test UI Code
    // fb = *info;
    // printf("fb_init, width = %d, height = %d, address = %lu .\n", fb.width, fb.height, fb.framebuffer);
    // uint32_t* fb = (uint32_t*)(info->framebuffer);

    // int w = info->width;
    // int h = info->height;
    // for (int y = 0; y < h; y++) {
    //     for (int x = 0; x < w; x++) {
    //         fb[y * info->pitch / 4 + x] = COLOR(x, y, x+y);
    //         // fb_put_pixel(x, y, COLOR(x, y, x+y));
    //     }
    // }
    fb = *info;

    fb_dev.width  = info->width;
    fb_dev.height = info->height;
    fb_dev.pitch  = info->pitch;
    fb_dev.bpp    = info->bpp;
    fb_dev.framebuffer = info->framebuffer;

    log_info("FB", "Framebuffer initialized: %ux%u, pitch=%u, bpp=%u, addr=0x%lx",
             fb_dev.width, fb_dev.height, fb_dev.pitch, fb_dev.bpp, fb_dev.framebuffer);


    // // Optional: Fill framebuffer with a test pattern
    // uint32_t* fb_mem = (uint32_t*)fb_dev.framebuffer;
    // for (int y = 0; y < fb_dev.height; y++) {
    //     for (int x = 0; x < fb_dev.width; x++) {
    //         fb_mem[y * (fb_dev.pitch / 4) + x] = (x ^ y) | ((x & y) << 8); // simple pattern
    //     }
    // }
    
    // Register /dev/fb0 in VFS
    VFS_RegisterDevice("/dev/fb0", &fb_fops, &fb_dev);
}

void fb_put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= fb_dev.width || y < 0 || y >= fb_dev.height)
        return;

    uint32_t* fb_mem = (uint32_t*)fb_dev.framebuffer;
    fb_mem[y * (fb_dev.pitch / 4) + x] = color;
}

void fb_clear(uint32_t color)
{
    for (int y = 0; y < fb.height; y++)
        for (int x = 0; x < fb.width; x++)
            fb_put_pixel(x, y, color);
}

void fb_draw_rect(int x, int y, int width, int height, uint32_t color)
{
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++)
            fb_put_pixel(x + i, y + j, color);
}

void test_fb(void)
{
    int fd = VFS_Open("/dev/fb0", 0);
    if (fd < 0) {
        log_error("FB", "Cannot open /dev/fb0");
        return;
    }

    uint32_t buffer[100 * 100];
    for (int i = 0; i < 100 * 100; i++)
        buffer[i] = 0x00FF00; // green

    VFS_Write(fd, buffer, sizeof(buffer));
    log_info("FB", "Wrote 100x100 green block to framebuffer");
}