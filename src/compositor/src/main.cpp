#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
// #include "compositor.h"
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <sys/epoll.h>     // struct epoll_event, EPOLLIN, EPOLL_CTL_ADD
#include <sys/syscall.h>
#include <wayland-server.h>


    struct fb_device {
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
        uint32_t bpp;
        uint64_t framebuffer;
    } info;



int main()
{
    printf("Hello from Compositor!\n");

    // int array[1000000];
    // printf("Checking stack page size. int array[1000000] Okay\n");

    // int* arrayPtr =new int[1000000];
    // printf("Checking heap page size.new int[1000000], Okay\n");


    wl_display* display = wl_display_create();
    std::cout << "[LumaCompositor] after wl_display_create.. " << std::endl;

    return 0;
}
// int main() {
//     printf("Hello from Compositor!\n");

//     wl_display* display = wl_display_create();
//     std::cout << "[LumaCompositor] after wl_display_create.. " << std::endl;


//     setenv("XKB_CONFIG_ROOT", "/usr/share/X11/xkb", 1);
//     setenv("XKB_LOG_LEVEL", "debug", 1);

//     int fb_fd = open("/dev/fb0", O_RDWR);
//     if (fb_fd < 0) {
//         perror("open /dev/fb0");
//         exit(1);
//     }

//     if (ioctl(fb_fd, 0x4602, &info) < 0) {
//         perror("ioctl FBIOGET_VSCREENINFO");
//         exit(1);
//     }

//     size_t fb_size = (size_t)info.pitch * info.height;
//     printf("fb_size is %zu (pitch=%u, height=%u)\n", fb_size, info.pitch, info.height);

//     if (fb_size == 0) {
//         fprintf(stderr, "fb_size is 0 (pitch=%u, height=%u)\n", info.pitch, info.height);
//         exit(1);
//     }
//     void *fb = mmap(NULL,
//                 fb_size,
//                 PROT_READ | PROT_WRITE,
//                 MAP_SHARED,
//                 fb_fd,
//                 0);
//     if (fb == MAP_FAILED) {
//         perror("ERROR: mmap fb Failed \0");
//         exit(1);
//     }

//     close(fb_fd);

//     printf("User-space test complete. Ready to launch compositor!\n");

//     // LumaCompositor comp{};
//     // if (!luma_init(&comp))
//     // {
//     //     fprintf(stderr, "LumaCompositor init failed, not running event loop\n");
//     //     return 1;
//     // }
    
//     // luma_run(&comp);

//     return 0;
// }


    // struct stat st;

    // int ret = stat("/usr/share/X11/xkb", &st);
    // printf("ret=%d errno=%d st_mode=%#o S_ISDIR=%d\n",
    //     ret, errno, st.st_mode, S_ISDIR(st.st_mode));

    // ret = stat("/usr/share/X11/xkb/rules/evdev", &st);
    // printf("ret=%d errno=%d st_mode=%#o S_ISREG=%d\n",
    //     ret, errno, st.st_mode, S_ISREG(st.st_mode));

    // int fd = open("/usr/share/X11/xkb/rules/evdev", O_RDONLY);
    // printf("open evdev fd=%d errno=%d\n", fd, errno);
    // if (fd < 0) {
    //     printf("Failed to open: /usr/share/X11/xkb/rules/evdev\n");
    // } else {
    //     printf("Opened evdev successfully\n");
    //     close(fd);
    // }

    // // Print xkb/types directory entries to verify we can read them
    // DIR *d = opendir("/usr/share/X11/xkb/types");
    // struct dirent *e;
    // while ((e = readdir(d))) {
    //     printf("entry: %s\n", e->d_name);
    // }



    // // uint64_t rbp_before = 0, rbp_after = 0;
    // // asm volatile("mov %%rbp, %0" : "=r"(rbp_before));
    // // syscall(SYS_getpid);
    // // asm volatile("mov %%rbp, %0" : "=r"(rbp_after));
    // // printf("rbp_before=%p rbp_after=%p\n", (void*)rbp_before, (void*)rbp_after);

    // // int fd = syscall(SYS_epoll_create1, 0);
    // // printf("epoll_create1 -> fd=%d\n", fd);
    // // struct epoll_event ev = { .events = EPOLLIN, .data = { .u32 = 123 } };
    // // int r = syscall(SYS_epoll_ctl, fd, EPOLL_CTL_ADD, 0, &ev);
    // // printf("epoll_ctl -> r=%d errno=%d\n", r, errno);

    // LumaCompositor comp{};
    // printf("LumaCompositor] starting wl_display_create.. \n");

    // comp.display = wl_display_create();
    // printf("LumaCompositor]  wl_display_create.. \n");
    // // std::cout << "[LumaCompositor]  wl_display_create.. " << std::endl;

    // if (!comp.display)
    // {
    //     printf("[LumaCompositor] Failed to create display \n");

    //     // std::cerr << "[LumaCompositor] Failed to create display\n";
    //     exit(1);
    //     return false;
    // }
    // else
    // {
    //     printf("[LumaCompositor] wl_display_create Done... \n");

    //     // std::cout << "[LumaCompositor] wl_display_create Done..." << std::endl;
    // }
