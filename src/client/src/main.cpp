#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

static const int W = 500, H = 300;
static wl_display* dpy; static wl_registry* reg;
static wl_compositor* comp; static wl_shm* shm; static xdg_wm_base* wm; static wl_seat* seat;
static wl_surface* surf; static xdg_surface* xs; static xdg_toplevel* tl;
static wl_buffer* buf; static uint32_t* px;
static int mx = W / 2, my = H / 2; static bool pressed = false, configured = false;

static void rect(int x, int y, int w, int h, uint32_t c) {
    for (int j = y; j < y + h; j++) for (int i = x; i < x + w; i++)
        if (i >= 0 && j >= 0 && i < W && j < H) px[j * W + i] = c;
}
static void draw() {
    for (int y = 0; y < H; y++) {
        uint32_t v = 0x20 + (y * 0x30) / H;
        rect(0, y, W, 1, 0xFF000000 | (v << 16) | (v << 8) | 0x60);
    }
    rect(0, 0, W, 28, 0xFF3A6EA5);                      // title bar
    rect(W - 28, 4, 20, 20, 0xFFCC3333);                // close box
    rect(40, 200, 160, 48, pressed ? 0xFF55CC88 : 0xFF2E8B57);   // button
    rect(mx - 6, my - 6, 12, 12, 0xFFFFFF00);           // follows the pointer
    wl_surface_attach(surf, buf, 0, 0);
    wl_surface_damage(surf, 0, 0, W, H);
    wl_surface_commit(surf);
}
static void make_buffer() {
    size_t stride = W * 4, size = stride * H;
    int fd = (int)syscall(SYS_memfd_create, "client-shm", 0);
    ftruncate(fd, size);
    px = (uint32_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    wl_shm_pool* pool = wl_shm_create_pool(shm, fd, size);
    buf = wl_shm_pool_create_buffer(pool, 0, W, H, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
}

/* pointer */
static void p_enter(void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t x, wl_fixed_t y) { mx = wl_fixed_to_int(x); my = wl_fixed_to_int(y); }
static void p_leave(void*, wl_pointer*, uint32_t, wl_surface*) {}
static void p_motion(void*, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y) { mx = wl_fixed_to_int(x); my = wl_fixed_to_int(y); if (configured) draw(); }
static void p_button(void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t state) { pressed = state == WL_POINTER_BUTTON_STATE_PRESSED; if (configured) draw(); }
static wl_pointer_listener pl;     // assigned by name: stays valid across protocol versions

static void seat_caps(void*, wl_seat* s, uint32_t caps) {
    if (caps & WL_SEAT_CAPABILITY_POINTER) {
        wl_pointer* p = wl_seat_get_pointer(s);
        pl.enter = p_enter; pl.leave = p_leave; pl.motion = p_motion; pl.button = p_button;
        wl_pointer_add_listener(p, &pl, nullptr);
    }
}
static void seat_name(void*, wl_seat*, const char*) {}
static const wl_seat_listener seat_l = { seat_caps, seat_name };

static void wm_ping(void*, xdg_wm_base* b, uint32_t s) { xdg_wm_base_pong(b, s); }
static const xdg_wm_base_listener wm_l = { wm_ping };
static void xs_conf(void*, xdg_surface* x, uint32_t serial) {
    xdg_surface_ack_configure(x, serial);
    if (!configured) { configured = true; draw(); }
}
static const xdg_surface_listener xs_l = { xs_conf };
static void tl_conf(void*, xdg_toplevel*, int32_t, int32_t, wl_array*) {}
static void tl_close(void*, xdg_toplevel*) { _exit(0); }
static const xdg_toplevel_listener tl_l = { tl_conf, tl_close };

static void reg_global(void*, wl_registry* r, uint32_t name, const char* i, uint32_t v) {
    if (!strcmp(i, wl_compositor_interface.name)) comp = (wl_compositor*)wl_registry_bind(r, name, &wl_compositor_interface, v < 4 ? v : 4);
    else if (!strcmp(i, wl_shm_interface.name))   shm  = (wl_shm*)wl_registry_bind(r, name, &wl_shm_interface, 1);
    else if (!strcmp(i, xdg_wm_base_interface.name)) { wm = (xdg_wm_base*)wl_registry_bind(r, name, &xdg_wm_base_interface, 1); xdg_wm_base_add_listener(wm, &wm_l, nullptr); }
    else if (!strcmp(i, wl_seat_interface.name))  { seat = (wl_seat*)wl_registry_bind(r, name, &wl_seat_interface, 4); wl_seat_add_listener(seat, &seat_l, nullptr); }
}
static void reg_remove(void*, wl_registry*, uint32_t) {}
static const wl_registry_listener reg_l = { reg_global, reg_remove };

int main() {

    std::cout<<"[client] Main started..."<<std::endl;
    dpy = wl_display_connect(nullptr);
    if (!dpy)
    {
        printf("[client] connect failed\n"); return 1;
    }
    else
    {
        std::cout<<"[client] Client connected..."<<std::endl;
    }
    reg = wl_display_get_registry(dpy);
    wl_registry_add_listener(reg, &reg_l, nullptr);
    wl_display_roundtrip(dpy);
    if (!comp || !shm || !wm)
    {
        printf("[client] missing globals\n");
        return 1;
    }

    make_buffer();
    surf = wl_compositor_create_surface(comp);
    xs = xdg_wm_base_get_xdg_surface(wm, surf);
    xdg_surface_add_listener(xs, &xs_l, nullptr);
    tl = xdg_surface_get_toplevel(xs);
    xdg_toplevel_add_listener(tl, &tl_l, nullptr);
    xdg_toplevel_set_title(tl, "Luma Client");
    wl_surface_commit(surf);

    while (wl_display_dispatch(dpy) != -1) {}
    return 0;
}