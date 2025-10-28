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


struct LumaCompositor {
    wl_display* display;
    wl_event_loop* loop;

    // Globals
    wl_global* compositor_global;
    wl_global* shm_global;
    wl_global* xdg_wm_base_global;
};

void luma_init(LumaCompositor* comp);
void luma_run(LumaCompositor* comp);