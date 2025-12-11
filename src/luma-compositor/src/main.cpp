
// int main() {
//     LumaCompositor comp{};
//     luma_init(&comp);
//     luma_run(&comp);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include "compositor.h"

// Optional: include your compositor header
#include "compositor.h"

int main() {
    printf("User-space environment ready!\n");

    // Example: open /dev/fb0 (just to test kernel FB access)
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        perror("Failed to open /dev/fb0");
        return 1;
    }
    printf("/dev/fb0 opened successfully.\n");

    // Map the framebuffer (example: first 8 MB)
    void *fb_ptr = mmap(NULL, 8*1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_ptr == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        close(fb_fd);
        return 1;
    }
    printf("Framebuffer mapped at %p\n", fb_ptr);

    // Optional: unmap & close
    munmap(fb_ptr, 8*1024*1024);
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
