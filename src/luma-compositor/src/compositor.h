// #pragma once
// extern "C" {
// #include <wayland-server.h>
// }
// #include <string>
// #include <stdexcept>

// class LumaCompositor {
// public:
//     LumaCompositor();
//     ~LumaCompositor();

//     void run();

// private:
//     struct wl_display* display = nullptr;
//     struct wl_event_loop* eventLoop = nullptr;
// };

#pragma once
#include <wayland-server.h>
#include <list>

// Forward declarations
struct my_output;

struct LumaCompositor {
    wl_display* display;
    wl_event_loop* loop;

    // Globals
    wl_global* compositor_global;
    wl_global* shm_global;
    wl_global* xdg_wm_base_global;
    wl_global* output_global;

    std::list<my_output*> client_outputs;
};

struct my_surface {
    wl_resource* resource = nullptr;
    // add fields you need (xdg surface pointer, buffers, etc.)

    wl_resource* xdg_surface_res = nullptr;
    wl_resource* toplevel_res = nullptr;
    wl_resource* buffer_res = nullptr;
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
    wl_resource* resource;
    int width;
    int height;
    int stride;
    uint32_t format;
};

void luma_init(LumaCompositor* comp);
void luma_run(LumaCompositor* comp);