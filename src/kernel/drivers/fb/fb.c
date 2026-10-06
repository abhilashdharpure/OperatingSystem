#include "fb.h"
#include "stdio.h"
#include "hal/vfs.h"
#include "debug.h"
#include "paging.h"

VbeModeInfo fb;
fb_device_t fb_dev;

extern uint64_t *kernel_pml4_virt;

static struct { uint64_t s, e; } g_shared_maps[64];
static void shared_map_add(uint64_t s, uint64_t len) {
    for (int i = 0; i < 64; i++) if (!g_shared_maps[i].e) { g_shared_maps[i].s = s; g_shared_maps[i].e = s + len; return; }
}
static bool is_shared_va(uint64_t va) {
    for (int i = 0; i < 64; i++) if (va >= g_shared_maps[i].s && va < g_shared_maps[i].e) return true;
    return false;
}

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


static int fb_mmap(struct file *f, uint64_t length, uint64_t prot,
                   uint64_t flags, uint64_t offset, uint64_t *out_user_va)
{
    fb_device_t *dev = f->private_data;
    uint64_t fb_size = (uint64_t)dev->pitch * dev->height;

    if (offset >= fb_size || offset + length > fb_size) return -1;
    if (!current_process) return -1;
    if (current_process->mmap_base == 0)
        current_process->mmap_base = USER_MMAP_BASE;

    uint64_t len   = (length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint64_t start = (current_process->mmap_base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uint64_t pte_flags = PAGE_PRESENT | PAGE_USER;
    if (prot & PROT_WRITE) pte_flags |= PAGE_RW;

    uint64_t kva_base = (dev->framebuffer + offset) & ~(PAGE_SIZE - 1);

    for (uint64_t off = 0; off < len; off += PAGE_SIZE) {
        uint64_t pa = get_mapped_phys(kernel_pml4_virt, kva_base + off);
        if (!pa) {
            log_error("FB", "fb_mmap: no phys for kva=0x%llx",
                      (unsigned long long)(kva_base + off));
            return -1;
        }
        if (map_page(current_process->page_directory, start + off,
                     pa & ~(PAGE_SIZE - 1), pte_flags) != 0)
            return -1;
    }

    shared_map_add(start, len);
    current_process->mmap_base = start + len;
    *out_user_va = start;
    return 0;
}

struct file_operations fb_fops = {
    .open  = fb_open,
    .read  = fb_read,
    .write = fb_write,
    .ioctl = fb_ioctl,
    .mmap  = fb_mmap,
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


    log_info("FB",
         "info->framebuffer = 0x%llx",
         (unsigned long long)info->framebuffer);

    log_info("FB",
            "fb_dev.framebuffer = 0x%llx",
            (unsigned long long)fb_dev.framebuffer);

    log_info("FB",
            "DIRECT_MAP_BASE = 0x%llx",
            (unsigned long long)DIRECT_MAP_BASE);

    log_info("FB",
            "KERNEL_VMA_BASE = 0x%llx",
            (unsigned long long)KERNEL_VMA_BASE);

            uint64_t fb_addr = info->framebuffer;

            if (fb_addr >= DIRECT_MAP_BASE) {
                log_info("FB",
                        "framebuffer looks like DIRECT-MAP virtual address; PA=0x%llx",
                        (unsigned long long)virt_to_phys((void *)fb_addr));
            } else if (fb_addr >= KERNEL_VMA_BASE) {
                log_info("FB",
                        "framebuffer looks like KERNEL virtual address; PA=0x%llx",
                        (unsigned long long)kernel_virt_to_phys((void *)fb_addr));
            } else {
                log_info("FB",
                        "framebuffer looks like a physical/low address");
            }

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