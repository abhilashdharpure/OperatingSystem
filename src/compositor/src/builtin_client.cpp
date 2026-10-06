#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "builtin_client.h"
#include <cerrno>

namespace {

struct Glyph { char c; uint8_t r[7]; };
const Glyph font[] = {
    {'H',{0x11,0x11,0x11,0x1F,0x11,0x11,0x11}},
    {'E',{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}},
    {'L',{0x10,0x10,0x10,0x10,0x10,0x10,0x1F}},
    {'O',{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}},
    {'U',{0x11,0x11,0x11,0x11,0x11,0x11,0x0E}},
    {'M',{0x11,0x1B,0x15,0x15,0x11,0x11,0x11}},
    {'A',{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}},
    {'S',{0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}},
};

struct Client {
    wl_display*    display    = nullptr;
    wl_registry*   registry   = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shm*        shm        = nullptr;
    xdg_wm_base*   wm         = nullptr;
    wl_surface*    surface    = nullptr;
    xdg_surface*   xsurf      = nullptr;
    xdg_toplevel*  toplevel   = nullptr;
    wl_buffer*     buffer     = nullptr;
    bool configured = false;
    int  w = 480, h = 320;
} c;

void put(uint32_t* px, int x, int y, uint32_t col) {
    if (x >= 0 && y >= 0 && x < c.w && y < c.h) px[y * c.w + x] = col;
}
void rect(uint32_t* px, int x, int y, int w, int h, uint32_t col) {
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) put(px, x + i, y + j, col);
}
void text(uint32_t* px, int x, int y, const char* s, int scale, uint32_t col) {
    for (; *s; s++, x += 6 * scale) {
        for (const Glyph& g : font) {
            if (g.c != *s) continue;
            for (int row = 0; row < 7; row++)
                for (int bit = 0; bit < 5; bit++)
                    if (g.r[row] & (0x10 >> bit))
                        rect(px, x + bit * scale, y + row * scale, scale, scale, col);
        }
    }
}

void paint(uint32_t* px) {
    for (int y = 0; y < c.h; y++) {                       /* vertical gradient */
        uint32_t v = 0x20 + (y * 0x30) / c.h;
        rect(px, 0, y, c.w, 1, 0xFF000000 | (v << 16) | (v << 8) | 0x60);
    }
    rect(px, 0, 0, c.w, 28, 0xFF3A6EA5);                  /* title bar */
    text(px, 10, 7, "LUMA OS", 2, 0xFFFFFFFF);
    rect(px, c.w - 28, 4, 20, 20, 0xFFCC3333);            /* close box */
    text(px, 40, 100, "HELLO", 8, 0xFFFFFFFF);
    rect(px, 40, 200, 160, 48, 0xFF2E8B57);               /* button */
    text(px, 62, 214, "OK", 3, 0xFFFFFFFF);
}

int make_shm(size_t size) {
    int fd = (int)syscall(SYS_memfd_create, "luma-shm", 0);
    if (fd < 0) return -1;
    if (ftruncate(fd, (off_t)size) < 0) { close(fd); return -1; }
    return fd;
}

void draw() {
    int stride = c.w * 4;
    size_t size = (size_t)stride * c.h;
    int fd = make_shm(size);
    if (fd < 0) { printf("[Client] memfd failed\n"); return; }

    void* data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { printf("[Client] mmap failed\n"); close(fd); return; }

    paint((uint32_t*)data);   /* NOTE: do not munmap, see notes below */

    wl_shm_pool* pool = wl_shm_create_pool(c.shm, fd, (int)size);
    c.buffer = wl_shm_pool_create_buffer(pool, 0, c.w, c.h, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    wl_surface_attach(c.surface, c.buffer, 0, 0);
    wl_surface_damage(c.surface, 0, 0, c.w, c.h);
    wl_surface_commit(c.surface);
    printf("[Client] first frame committed\n");
}

/* xdg_wm_base */
void wm_ping(void*, xdg_wm_base* wm, uint32_t serial) { xdg_wm_base_pong(wm, serial); }
const xdg_wm_base_listener wm_listener = { wm_ping };

/* xdg_surface */
void xs_configure(void*, xdg_surface* xs, uint32_t serial) {
    xdg_surface_ack_configure(xs, serial);
    if (!c.configured) { c.configured = true; draw(); }
}
const xdg_surface_listener xs_listener = { xs_configure };

/* xdg_toplevel */
void tl_configure(void*, xdg_toplevel*, int32_t w, int32_t h, wl_array*) {
    if (!c.configured && w > 0 && h > 0) { c.w = w; c.h = h; }
}
void tl_close(void*, xdg_toplevel*) { printf("[Client] close requested\n"); }
const xdg_toplevel_listener tl_listener = { tl_configure, tl_close };

void create_window() {
    if (c.surface || !c.compositor || !c.shm || !c.wm) return;
    c.surface  = wl_compositor_create_surface(c.compositor);
    c.xsurf    = xdg_wm_base_get_xdg_surface(c.wm, c.surface);
    xdg_surface_add_listener(c.xsurf, &xs_listener, nullptr);
    c.toplevel = xdg_surface_get_toplevel(c.xsurf);
    xdg_toplevel_add_listener(c.toplevel, &tl_listener, nullptr);
    xdg_toplevel_set_title(c.toplevel, "Luma Demo");
    xdg_toplevel_set_app_id(c.toplevel, "luma.demo");
    wl_surface_commit(c.surface);   /* initial empty commit -> compositor sends configure */
}

/* registry */
void reg_global(void*, wl_registry* r, uint32_t name, const char* iface, uint32_t ver) {
    printf("[Client] global %u: %s v%u\n", name, iface, ver);
    if (!strcmp(iface, wl_compositor_interface.name)) {
        c.compositor = (wl_compositor*)wl_registry_bind(r, name, &wl_compositor_interface, ver < 4 ? ver : 4);
    } else if (!strcmp(iface, wl_shm_interface.name)) {
        c.shm = (wl_shm*)wl_registry_bind(r, name, &wl_shm_interface, 1);
    } else if (!strcmp(iface, xdg_wm_base_interface.name)) {
        c.wm = (xdg_wm_base*)wl_registry_bind(r, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(c.wm, &wm_listener, nullptr);
    }
    create_window();
}
void reg_remove(void*, wl_registry*, uint32_t) {}
const wl_registry_listener reg_listener = { reg_global, reg_remove };

} // namespace

extern "C" int builtin_client_start(void) {
    c.display = wl_display_connect(nullptr);     /* uses $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY */
    if (!c.display) { printf("[Client] wl_display_connect failed\n"); return -1; }
    c.registry = wl_display_get_registry(c.display);
    printf("[Client] registry requested\n");
    wl_registry_add_listener(c.registry, &reg_listener, nullptr);

    printf("[Client] flushing display\n");
    wl_display_flush(c.display);
    printf("[Client] connected, fd=%d\n", wl_display_get_fd(c.display));
    return wl_display_get_fd(c.display);
}

extern "C" void builtin_client_pump(void) {
    if (!c.display) { printf("[Pump] no display\n"); return; }

    int err = wl_display_get_error(c.display);
    if (err) { printf("[Pump] display error=%d\n", err); return; }

    int n = 0;
    while (wl_display_prepare_read(c.display) != 0) {
        n = wl_display_dispatch_pending(c.display);
        printf("[Pump] dispatch_pending -> %d\n", n);
    }
    int f = wl_display_flush(c.display);
    int r = wl_display_read_events(c.display);
    printf("[Pump] flush=%d read_events=%d errno=%d\n", f, r, errno);
    n = wl_display_dispatch_pending(c.display);
    printf("[Pump] dispatched=%d\n", n);
    wl_display_flush(c.display);
}

extern "C" int builtin_client_get_error()
{
    if (!c.display)
        return -1;

    return wl_display_get_error(c.display);
}

extern "C" void builtin_client_disconnect()
{
    if (!c.display)
        return;

    wl_display_disconnect(c.display);
}