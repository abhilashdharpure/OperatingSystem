#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include "compositor.h"
#include <sys/ioctl.h>
#include <sys/stat.h>

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

    struct stat st;

    int ret = stat("/usr/share/X11/xkb", &st);
    printf("ret=%d errno=%d st_mode=%#o S_ISDIR=%d\n",
        ret, errno, st.st_mode, S_ISDIR(st.st_mode));

    ret = stat("/usr/share/X11/xkb/rules/evdev", &st);
    printf("ret=%d errno=%d st_mode=%#o S_ISREG=%d\n",
        ret, errno, st.st_mode, S_ISREG(st.st_mode));

    int fd = open("/usr/share/X11/xkb/rules/evdev", O_RDONLY);
    printf("open evdev fd=%d errno=%d\n", fd, errno);
    if (fd < 0) {
        printf("Failed to open: /usr/share/X11/xkb/rules/evdev\n");
    } else {
        printf("Opened evdev successfully\n");
        close(fd);
    }


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
    printf("fb_size is %zu (pitch=%u, height=%u)\n", fb_size, info.pitch, info.height);

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
