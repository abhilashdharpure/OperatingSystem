#pragma once
#include <slint-platform.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <span>
#include <chrono>

struct WlGlobals {
    wl_display* dpy = nullptr; wl_registry* reg = nullptr;
    wl_compositor* comp = nullptr; wl_shm* shm = nullptr;
    xdg_wm_base* wm = nullptr; wl_seat* seat = nullptr;
};
inline WlGlobals g;

class LumaWindow;

inline LumaWindow* g_win = nullptr;

inline uint32_t g_last_serial = 0;
// ---------- input ----------
inline float g_px = 0, g_py = 0;

class LumaWindow : public slint::platform::WindowAdapter {
public:
    slint::platform::SoftwareRenderer renderer_{
        slint::platform::SoftwareRenderer::RepaintBufferType::NewBuffer};
    slint::PhysicalSize size_{{600, 400}};
    bool dirty_ = true, configured_ = false, frame_pending_ = false, closed_ = false;
    bool maximized_ = false;

    wl_surface* surf_ = nullptr; xdg_surface* xs_ = nullptr; xdg_toplevel* tl_ = nullptr;
    wl_buffer* buf_ = nullptr; uint32_t* shm_px_ = nullptr; size_t shm_size_ = 0;
    std::vector<slint::Rgb8Pixel> scratch_;

    // --- WindowAdapter interface ---
    slint::platform::AbstractRenderer& renderer() override { return renderer_; }
    slint::PhysicalSize size() override { return size_; }
    void request_redraw() override { dirty_ = true; }
    void set_visible(bool v) override { if (v && !surf_) create_surface(); }

    // --- Wayland setup ---
    void make_buffer() {
        int w = size_.width, h = size_.height;
        if (buf_) wl_buffer_destroy(buf_);
        shm_size_ = (size_t)w * h * 4;
        int fd = (int)syscall(SYS_memfd_create, "slint-shm", 0);
        ftruncate(fd, shm_size_);
        shm_px_ = (uint32_t*)mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        wl_shm_pool* pool = wl_shm_create_pool(g.shm, fd, shm_size_);
        buf_ = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_XRGB8888);
        wl_shm_pool_destroy(pool);
        close(fd);
        scratch_.assign((size_t)w * h, slint::Rgb8Pixel{});
        // renderer_.set_repaint_buffer_type(slint::platform::SoftwareRenderer::RepaintBufferType::NewBuffer);
        // renderer_.set_repaint_buffer_type(slint::platform::SoftwareRenderer::RepaintBufferType::ReusedBuffer);
        dirty_ = true;
    }

    void create_surface() {
        static const xdg_surface_listener xs_l = { [](void* d, xdg_surface* x, uint32_t serial) {
            xdg_surface_ack_configure(x, serial);
            auto* self = (LumaWindow*)d;
            if (!self->configured_) { self->configured_ = true; self->dirty_ = true; }
        }};
        static const xdg_toplevel_listener tl_l = {
            // [](void* d, xdg_toplevel*, int32_t w, int32_t h, wl_array*) {
            //     auto* self = (LumaWindow*)d;
            //     if (w > 0 && h > 0 && ((uint32_t)w != self->size_.width || (uint32_t)h != self->size_.height)) {
            //         self->size_ = slint::PhysicalSize({(uint32_t)w, (uint32_t)h});
            //         self->make_buffer();
            //         self->window().set_size(self->size_);
            //     }
            // },

            [](void* d, xdg_toplevel*, int32_t w, int32_t h, wl_array* states) {
                auto* self = (LumaWindow*)d;
                self->maximized_ = false;
                for (uint32_t* s = (uint32_t*)states->data;
                    (char*)s < (char*)states->data + states->size; s++)
                    if (*s == XDG_TOPLEVEL_STATE_MAXIMIZED) self->maximized_ = true;
                if (w > 0 && h > 0) {
                    self->resize_to((uint32_t)w, (uint32_t)h);
                    self->window().set_size(self->size_);
                }
            },
            // [](void* d, xdg_toplevel*, int32_t w, int32_t h, wl_array*) {
            //     auto* self = (LumaWindow*)d;
            //     if (w > 0 && h > 0) {
            //         self->resize_to((uint32_t)w, (uint32_t)h);
            //         self->window().set_size(self->size_);
            //     }
            // },
            [](void* d, xdg_toplevel*) { ((LumaWindow*)d)->closed_ = true; }
        };
        surf_ = wl_compositor_create_surface(g.comp);
        xs_ = xdg_wm_base_get_xdg_surface(g.wm, surf_);
        xdg_surface_add_listener(xs_, &xs_l, this);
        tl_ = xdg_surface_get_toplevel(xs_);
        xdg_toplevel_add_listener(tl_, &tl_l, this);
        xdg_toplevel_set_title(tl_, "Luma Slint");
        make_buffer();
        wl_surface_commit(surf_);
    }
    void resize_to(uint32_t w, uint32_t h) {
        if (buf_ && w == size_.width && h == size_.height) return;
        size_ = slint::PhysicalSize({w, h});
        make_buffer();                 // also reallocates scratch_ from size_
    }

    // Slint asks the adapter to resize (preferred size, layout constraints). Match signature to the header.
    void set_size(slint::PhysicalSize size) override{
        resize_to(size.width, size.height);
    }

    void draw() {
        if (!configured_ || frame_pending_ || !buf_) return;
        dirty_ = false;


        auto ws = window().size();                        // what Slint will render at
        if (ws.width != size_.width || ws.height != size_.height)
            resize_to(ws.width, ws.height);

        dirty_ = false;
        int w = size_.width, h = size_.height;
        renderer_.render(std::span<slint::Rgb8Pixel>(scratch_), w);

        // int w = size_.width, h = size_.height;
        // renderer_.render(std::span<slint::Rgb8Pixel>(scratch_), w);
        // Rgb8 -> XRGB8888 (whole frame; optimize later using the returned dirty region)
        for (size_t i = 0, n = (size_t)w * h; i < n; i++) {
            const auto& p = scratch_[i];
            shm_px_[i] = 0xFF000000u | (p.r << 16) | (p.g << 8) | p.b;
        }
        static const wl_callback_listener cb_l = { [](void* d, wl_callback* cb, uint32_t) {
            wl_callback_destroy(cb); ((LumaWindow*)d)->frame_pending_ = false;
        }};
        wl_callback* cb = wl_surface_frame(surf_);
        wl_callback_add_listener(cb, &cb_l, this);
        frame_pending_ = true;
        wl_surface_attach(surf_, buf_, 0, 0);
        wl_surface_damage(surf_, 0, 0, w, h);
        wl_surface_commit(surf_);
    }

    void begin_move() {
        if (!tl_ || !g.seat) return;
        xdg_toplevel_move(tl_, g.seat, g_last_serial);


        /*button*/ [](void*, wl_pointer*, uint32_t serial, uint32_t, uint32_t btn, uint32_t state) {
        g_last_serial = serial;
        auto b = btn == 0x111 ? slint::PointerEventButton::Right
            : btn == 0x112 ? slint::PointerEventButton::Middle : slint::PointerEventButton::Left;
        slint::LogicalPosition p({g_px, g_py});
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) g_win->window().dispatch_pointer_press_event(p, b);
        else g_win->window().dispatch_pointer_release_event(p, b); },
        
        // the compositor owns the pointer during the grab: cancel the press in Slint
        window().dispatch_pointer_release_event(slint::LogicalPosition({g_px, g_py}),
                                                slint::PointerEventButton::Left);
        window().dispatch_pointer_exit_event();
    }
    void toggle_maximize() {
        if (!tl_) return;
        if (maximized_) xdg_toplevel_unset_maximized(tl_);
        else            xdg_toplevel_set_maximized(tl_);
    }
    void minimize()     { if (tl_) xdg_toplevel_set_minimized(tl_); }
    void close_window() { closed_ = true; }
};


inline xkb_context* g_xkb = nullptr; inline xkb_keymap* g_km = nullptr; inline xkb_state* g_xs = nullptr;

inline void ptr_noop() {}
inline const wl_pointer_listener ptr_l = {
    /*enter*/  [](void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t x, wl_fixed_t y) {
        g_px = wl_fixed_to_double(x); g_py = wl_fixed_to_double(y);
        g_win->window().dispatch_pointer_move_event(slint::LogicalPosition({g_px, g_py})); },
    /*leave*/  [](void*, wl_pointer*, uint32_t, wl_surface*) { g_win->window().dispatch_pointer_exit_event(); },
    /*motion*/ [](void*, wl_pointer*, uint32_t, wl_fixed_t x, wl_fixed_t y) {
        g_px = wl_fixed_to_double(x); g_py = wl_fixed_to_double(y);
        g_win->window().dispatch_pointer_move_event(slint::LogicalPosition({g_px, g_py})); },
    /*button*/ [](void*, wl_pointer*, uint32_t, uint32_t, uint32_t btn, uint32_t state) {
        auto b = btn == 0x111 ? slint::PointerEventButton::Right
               : btn == 0x112 ? slint::PointerEventButton::Middle : slint::PointerEventButton::Left;
        slint::LogicalPosition p({g_px, g_py});
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) g_win->window().dispatch_pointer_press_event(p, b);
        else g_win->window().dispatch_pointer_release_event(p, b); },
    /*axis*/          [](void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {},
    /*frame*/         [](void*, wl_pointer*) {},
    /*axis_source*/   [](void*, wl_pointer*, uint32_t) {},
    /*axis_stop*/     [](void*, wl_pointer*, uint32_t, uint32_t) {},
    /*axis_discrete*/ [](void*, wl_pointer*, uint32_t, int32_t) {},
};

inline const wl_keyboard_listener kbd_l = {
    /*keymap*/ [](void*, wl_keyboard*, uint32_t, int32_t fd, uint32_t size) {
        void* m = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (m == MAP_FAILED) { close(fd); return; }
        if (!g_xkb) g_xkb = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        if (g_xs) xkb_state_unref(g_xs);
        if (g_km) xkb_keymap_unref(g_km);
        g_km = xkb_keymap_new_from_buffer(g_xkb, (const char*)m, size - 1,
                                          XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        g_xs = g_km ? xkb_state_new(g_km) : nullptr;
        munmap(m, size); close(fd); },
    /*enter*/ [](void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*) {},
    /*leave*/ [](void*, wl_keyboard*, uint32_t, wl_surface*) {},
    /*key*/   [](void*, wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
        if (!g_xs) return;
        xkb_keysym_t sym = xkb_state_key_get_one_sym(g_xs, key + 8);
        bool down = state == WL_KEYBOARD_KEY_STATE_PRESSED;
        slint::SharedString text;
        char buf[16] = {0};
        switch (sym) {
            case XKB_KEY_Return: case XKB_KEY_KP_Enter: text = "\n"; break;
            case XKB_KEY_BackSpace: text = "\b"; break;
            case XKB_KEY_Tab: text = "\t"; break;
            case XKB_KEY_Escape: text = "\x1b"; break;
            case XKB_KEY_Delete: text = "\x7f"; break;
            case XKB_KEY_Left:  text = "\xEF\x9C\x82"; break;   // U+F702
            case XKB_KEY_Right: text = "\xEF\x9C\x83"; break;   // U+F703
            case XKB_KEY_Up:    text = "\xEF\x9C\x80"; break;   // U+F700
            case XKB_KEY_Down:  text = "\xEF\x9C\x81"; break;   // U+F701
            default:
                if (xkb_state_key_get_utf8(g_xs, key + 8, buf, sizeof(buf)) > 0) text = buf;
        }
        xkb_state_update_key(g_xs, key + 8, down ? XKB_KEY_DOWN : XKB_KEY_UP);
        if (text.size() == 0) return;
        if (down) g_win->window().dispatch_key_press_event(text);
        else      g_win->window().dispatch_key_release_event(text); },
    /*modifiers*/ [](void*, wl_keyboard*, uint32_t, uint32_t d, uint32_t l, uint32_t lk, uint32_t grp) {
        if (g_xs) xkb_state_update_mask(g_xs, d, l, lk, 0, 0, grp); },
    /*repeat_info*/ [](void*, wl_keyboard*, int32_t, int32_t) {},
};

inline const wl_seat_listener seat_l = {
    [](void*, wl_seat* s, uint32_t caps) {
        if (caps & WL_SEAT_CAPABILITY_POINTER)  wl_pointer_add_listener(wl_seat_get_pointer(s), &ptr_l, nullptr);
        if (caps & WL_SEAT_CAPABILITY_KEYBOARD) wl_keyboard_add_listener(wl_seat_get_keyboard(s), &kbd_l, nullptr);
    },
    [](void*, wl_seat*, const char*) {},
};

inline const xdg_wm_base_listener wm_l = { [](void*, xdg_wm_base* b, uint32_t s) { xdg_wm_base_pong(b, s); } };
inline const wl_registry_listener reg_l = {
    [](void*, wl_registry* r, uint32_t name, const char* i, uint32_t v) {
        if (!strcmp(i, wl_compositor_interface.name)) g.comp = (wl_compositor*)wl_registry_bind(r, name, &wl_compositor_interface, v < 4 ? v : 4);
        else if (!strcmp(i, wl_shm_interface.name))   g.shm  = (wl_shm*)wl_registry_bind(r, name, &wl_shm_interface, 1);
        else if (!strcmp(i, xdg_wm_base_interface.name)) { g.wm = (xdg_wm_base*)wl_registry_bind(r, name, &xdg_wm_base_interface, 1); xdg_wm_base_add_listener(g.wm, &wm_l, nullptr); }
        else if (!strcmp(i, wl_seat_interface.name))  { g.seat = (wl_seat*)wl_registry_bind(r, name, &wl_seat_interface, 4); wl_seat_add_listener(g.seat, &seat_l, nullptr); }
    },
    [](void*, wl_registry*, uint32_t) {},
};

// ---------- platform ----------
class LumaPlatform : public slint::platform::Platform {
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
    bool quit_ = false;
public:
    bool connect() {
        for (int i = 0; i < 200 && !(g.dpy = wl_display_connect(nullptr)); i++) sched_yield();
        if (!g.dpy) return false;
        g.reg = wl_display_get_registry(g.dpy);
        wl_registry_add_listener(g.reg, &reg_l, nullptr);
        wl_display_roundtrip(g.dpy);
        wl_display_roundtrip(g.dpy);   // second round: seat capabilities
        return g.comp && g.shm && g.wm;
    }
    std::unique_ptr<slint::platform::WindowAdapter> create_window_adapter() override {
        auto w = std::make_unique<LumaWindow>();
        g_win = w.get();
        return w;
    }
#ifdef SLINT_FEATURE_FREESTANDING
    std::chrono::milliseconds duration_since_start() override {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_);
    }
#endif
    void quit_event_loop() override { quit_ = true; }
    void run_event_loop() override {
        while (!quit_ && !(g_win && g_win->closed_)) {
            slint::platform::update_timers_and_animations();
            if (g_win) {
                if (g_win->window().has_active_animations()) g_win->dirty_ = true;
                if (g_win->dirty_) g_win->draw();
            }
            int timeout = -1;
            if (auto d = slint::platform::duration_until_next_timer_update()) timeout = (int)d->count();
            if (g_win && (g_win->dirty_ || g_win->window().has_active_animations())) timeout = 16;

            while (wl_display_prepare_read(g.dpy) != 0) wl_display_dispatch_pending(g.dpy);
            wl_display_flush(g.dpy);
            pollfd p{wl_display_get_fd(g.dpy), POLLIN, 0};
            int r = poll(&p, 1, timeout);
            if (r > 0) wl_display_read_events(g.dpy); else wl_display_cancel_read(g.dpy);
            wl_display_dispatch_pending(g.dpy);
        }
    }
};
