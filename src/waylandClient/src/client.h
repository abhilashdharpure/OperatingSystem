#pragma once

#include <string>
#include <wayland-client.h>



// /* Wayland code */
// struct client_state {
//     /* Globals */
//     struct wl_display *wl_display;
//     struct wl_registry *wl_registry;
//     struct wl_shm *wl_shm;
//     struct wl_compositor *wl_compositor;
//     struct xdg_wm_base *xdg_wm_base;
//     /* Objects */
//     struct wl_surface *wl_surface;
//     struct xdg_surface *xdg_surface;
//     struct xdg_toplevel *xdg_toplevel;
// };


class Client
{
public:

    Client(const std::string& displayName);
    ~Client();

    void Initialize();




private:
    const std::string& displayName;
    struct wl_display* display;
};