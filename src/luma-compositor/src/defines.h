#pragma once

#include <wayland-server.h>
#include <list>

// Forward declarations
struct my_output;
struct LumaSeat;
struct my_surface;

struct LumaCompositor {
    wl_display* display;
    wl_event_loop* loop;

    // Globals
    wl_global* compositor_global;
    wl_global* shm_global;
    wl_global* xdg_wm_base_global;
    wl_global* output_global;
    wl_global* seat_global;

    int fb_stride; 

    // list of mapped surfaces (top-to-bottom order matters later)
    std::vector<my_surface*> surfaces;

    // focus tracking
    wl_resource* focused_surface = nullptr; // wl_surface resource that's keyboard focused
    wl_resource* pointer_focused_surface = nullptr; // wl_surface resource that pointer currently over
    wl_resource *keyboard_resource = nullptr;
    wl_resource* wm_base_resource;

    LumaSeat* seat;
    
    std::list<my_output*> client_outputs;

    double cursor_x = 0.0;
    double cursor_y = 0.0;
    int output_width =  1200; //1920;
    int output_height = 800; //1080;

    int32_t focus_x = 0;
    int32_t focus_y = 0;
    int32_t focus_width = 0;
    int32_t focus_height = 0;

    struct xkb_context* xkb_ctx = nullptr;
    struct xkb_keymap* keymap = nullptr;
    struct xkb_state* xkb_state = nullptr;
};

struct my_surface
{
    wl_resource* resource = nullptr;
    wl_resource* xdg_surface_res = nullptr;
    wl_resource* toplevel_res = nullptr;
    LumaCompositor* compositor;

    // track attached buffer, pending callback, etc.
    wl_resource* buffer_res = nullptr;
    wl_resource* pending_callback = nullptr;

    // geometry we will use for hit-testing and configure
    int32_t x = 0, y = 0;   // top-left position on the compositor output
    int32_t width = 0, height = 0; // size the compositor gave via xdg_toplevel_send_configure
    bool configured = false;     
    bool is_xdg_toplevel = false;             // true when xdg_toplevel created for this surface
};

struct my_output {
    wl_resource* resource = nullptr;
    LumaCompositor* state = nullptr;

    int width = 1920;
    int height = 1080;
    int scale = 1;
    int refresh_rate = 60000;
    // You can extend with position, transform, etc.
};

struct shm_pool_data {
    int fd;
    size_t size;
    void* data;
};

struct shm_buffer {
    wl_resource* resource = nullptr;
    void* data = nullptr;
    size_t size = 0;
    int width = 0,
    height = 0,
    stride = 0;
    uint32_t format = 0;

    my_surface* owner_surface = nullptr;
};

struct LumaSeat {
    wl_global* seat_global;
    wl_list keyboards; // linked list of wl_resources
    wl_list pointers;
    wl_display* display;
    LumaCompositor* compositor;
};

struct keymap_fd_holder {
    int fd;
    struct wl_listener listener;
};
