#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include "compositor.h"
#include <sys/ioctl.h>

// Optional: include your compositor header
#include "compositor.h"

    struct fb_device {
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
        uint32_t bpp;
        uint64_t framebuffer;
    } info;


int main() {
    printf("Hello from Compositor!\n");

    // // Example: open /dev/fb0 (just to test kernel FB access)
    // int fb_fd = open("/dev/fb0", O_RDWR);
    // if (fb_fd < 0) {
    //     perror("Failed to open /dev/fb0\n");
    //     return 1;
    // }
    // printf("/dev/fb0 opened successfully.\n");

    // // Map the framebuffer (example: first 8 MB)
    // void *fb_ptr = mmap(NULL, 8*1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    // if (fb_ptr == MAP_FAILED) {
    //     perror("Failed to mmap framebuffer\n");
    //     close(fb_fd);
    //     return 1;
    // }

    // printf("Framebuffer mapped at %p\n", fb_ptr);
    // // Optional: unmap & close
    // munmap(fb_ptr, 8*1024*1024);



    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("open /dev/fb0");
        exit(1);
    }

    if (ioctl(fb_fd, 0x4602, &info) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        exit(1);
    }

    size_t fb_size = (size_t)info.pitch * info.height;
    if (fb_size == 0) {
        fprintf(stderr, "fb_size is 0 (pitch=%u, height=%u)\n", info.pitch, info.height);
        exit(1);
    }
    void *fb = mmap(NULL,
                fb_size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fb_fd,
                0);
    if (fb == MAP_FAILED) {
        perror("ERROR: mmap fb Failed \0");
        exit(1);
    }

    close(fb_fd);

    printf("User-space test complete. Ready to launch compositor!\n");

    // Here you can call your compositor main function
    // compositor_run();
    LumaCompositor comp{};
    luma_init(&comp);
    luma_run(&comp);

    // xyz
    return 0;
}
