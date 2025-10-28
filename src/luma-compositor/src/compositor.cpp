#include "compositor.h"
#include "wayland_protocols.h"
#include <iostream>
#include <wayland-server.h>
#include "build/generated/xdg-shell-protocol.h"

// --- wl_compositor bind ---
void bind_compositor(struct wl_client* client, void* data, uint32_t version, uint32_t id) {
    (void)data;
    wl_resource* res = wl_resource_create(client, &wl_compositor_interface, version, id);
    if (!res)
        wl_client_post_no_memory(client);
}

// --- wl_shm bind ---
void bind_shm(struct wl_client* client, void* data, uint32_t version, uint32_t id) {
    (void)data;
    wl_resource* res = wl_resource_create(client, &wl_shm_interface, version, id);
    if (!res)
        wl_client_post_no_memory(client);
}

// --- xdg_wm_base bind ---
void bind_xdg_wm_base(struct wl_client* client, void* data, uint32_t version, uint32_t id) {
    (void)data;
    wl_resource* res = wl_resource_create(client, &xdg_wm_base_interface, version, id);
    if (!res) {
        wl_client_post_no_memory(client);
        return;
    }

    // No implementation struct needed — only client sends requests like "pong"
    wl_resource_set_implementation(res, nullptr, nullptr, nullptr);
}

// --- Initialize compositor ---
void luma_init(LumaCompositor* comp) {
    comp->display = wl_display_create();
    if (!comp->display) {
        std::cerr << "[LumaCompositor] Failed to create display\n";
        exit(1);
    }

    wl_display_add_socket(comp->display, "luma-0");

    comp->loop = wl_display_get_event_loop(comp->display);

    comp->compositor_global = wl_global_create(comp->display, &wl_compositor_interface, 4, nullptr, bind_compositor);
    comp->shm_global = wl_global_create(comp->display, &wl_shm_interface, 1, nullptr, bind_shm);
    comp->xdg_wm_base_global = wl_global_create(comp->display, &xdg_wm_base_interface, 1, nullptr, bind_xdg_wm_base);

    std::cout << "[LumaCompositor] Wayland display initialized\n";
}

// --- Run compositor ---
void luma_run(LumaCompositor* comp) {
    std::cout << "[LumaCompositor] Running Wayland event loop...\n";
    wl_display_run(comp->display);
}
