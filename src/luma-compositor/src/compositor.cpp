#include "compositor.h"
#include <iostream>
#include <wayland-server.h>

// #include "build/generated/xdg-shell-protocol.h"
// #include "build/generated/wayland-protocol.h"
#include "xdg-shell-protocol.h"
#include "wayland-protocol.h"
#include <unistd.h>   // for close()
#include <sys/mman.h>
#include <wayland-server-core.h>

// --- Simple linked list utilities using std::list ---
static void add_to_list(std::list<my_output*>& lst, my_output* item)
{
    lst.push_back(item);
}

static void remove_from_list(std::list<my_output*>& lst, my_output* item)
{
    lst.remove(item);
}

static void buffer_destroy_cb(struct wl_client*, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] buffer_destroy\n";

    auto* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(resource));
    if (!buf)
    {
        return;
    }
    delete buf;
}

static const struct wl_buffer_interface buffer_impl = {
    .destroy = buffer_destroy_cb
};

static void buffer_destroy(struct wl_resource *resource)
{
    std::cout << "[LumaCompositor] buffer_destroy\n";

    auto* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(resource));
    if (!buf)
    {
        return;
    }
    delete buf;
}

static void shm_pool_resize(struct wl_client* client, struct wl_resource* resource, int32_t size)
{
    (void)client;
    // Just log — resizing is not yet implemented, but don’t crash
    std::cout << "[LumaCompositor] wl_shm_pool_resize(" << size << ")\n";
}

static void shm_pool_destroy_req(struct wl_client* client, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] shm_pool_destroy_req\n";

    (void)client;
    wl_resource_destroy(resource);
}


static void shm_pool_create_buffer(struct wl_client *client, struct wl_resource *pool_res,
                                   uint32_t id, int32_t offset, int32_t width, int32_t height,
                                   int32_t stride, uint32_t format)
{
    std::cout << "[LumaCompositor] shm_pool_create_buffer\n";

    // Create wl_buffer resource
    wl_resource *buf_res = wl_resource_create(client, &wl_buffer_interface,
                                              wl_resource_get_version(pool_res), id);
    if (!buf_res)
    {
        wl_client_post_no_memory(client);
        return;
    }

    shm_buffer* buf = new shm_buffer;
    buf->resource = buf_res;
    buf->width = width;
    buf->height = height;
    buf->stride = stride;
    buf->format = format;

    wl_resource_set_implementation(buf_res, &buffer_impl, buf, buffer_destroy);

    std::cout << "[LumaCompositor] End shm_pool_create_buffer\n";

}

static const struct wl_shm_pool_interface shm_pool_impl = {
    .create_buffer = shm_pool_create_buffer, // we'll fill later
    .destroy = shm_pool_destroy_req,
    .resize = shm_pool_resize,
};


// ------------------ wl_shm ------------------
static void shm_pool_destroy(struct wl_resource* resource)
{
    auto *pool = static_cast<shm_pool_data*>(wl_resource_get_user_data(resource));
    if (!pool) return;
    // If you mmap'ed pool->data, munmap here:
    if (pool->data && pool->data != MAP_FAILED) munmap(pool->data, pool->size);
    delete pool;
}

static void shm_create_pool(struct wl_client* client, struct wl_resource* resource,
                            uint32_t id, int32_t fd, int32_t size)
{
    std::cout << "[LumaCompositor] shm_create_pool\n";

   auto pool_res = wl_resource_create(client, &wl_shm_pool_interface, wl_resource_get_version(resource), id);
    if (!pool_res)
    {
        wl_client_post_no_memory(client);
        close(fd);
        return;
    }

    // Allocate structure to keep track of the shm region
    auto* pool = new shm_pool_data();
    pool->fd = fd;
    pool->size = size;

    pool->data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pool->data == MAP_FAILED) {
        std::cerr << "[LumaCompositor] mmap failed for shm pool\n";
        close(fd);
        wl_resource_destroy(pool_res);
        delete pool;
        return;
    }

    wl_resource_set_implementation(pool_res, &shm_pool_impl, pool, shm_pool_destroy);
    std::cout << "[LumaCompositor] End shm_create_pool\n";

}

static const struct wl_shm_interface shm_impl = {
    .create_pool = shm_create_pool
};


// ------------------ wl_surface ------------------
static void surface_attach(wl_client* /*client*/, wl_resource* surface_res, wl_resource* buffer, int32_t /*x*/, int32_t /*y*/)
{
    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));

    if (!surf)
    {
        return;
    }
    surf->buffer_res = buffer; // just track it
}

static void surface_commit(wl_client*, wl_resource* surface_res)
{
    std::cout << "[LumaCompositor] wl_surface commit\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));
    if (!surf)
    {
        return;
    }

    // Send initial xdg configure if exists
    if (surf->xdg_surface_res)
    {
        // wl_display *d = wl_resource_get_display(surf->xdg_surface_res);

        wl_client *client = wl_resource_get_client(surf->xdg_surface_res);
        wl_display *d = wl_client_get_display(client);


        uint32_t serial = wl_display_next_serial(d);
        xdg_surface_send_configure(surf->xdg_surface_res, serial);
    }
}

static void surface_destroy(wl_client* /*client*/, wl_resource* resource)
{
    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    delete surf;
}

static void surface_damage(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] surface_damage \n";

}

static void surface_frame(wl_client*, wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] surface_frame \n";

}

static void surface_set_opaque_region(wl_client*, wl_resource*, wl_resource*)
{
    std::cout << "[LumaCompositor] surface_set_opaque_region \n";
}

static void surface_set_input_region(wl_client*, wl_resource*, wl_resource*)
{
    std::cout << "[LumaCompositor] surface_set_input_region \n";
}

static void surface_set_buffer_transform(wl_client *client, wl_resource *resource, int32_t transform)
{
    std::cout << "[LumaCompositor] surface_set_buffer_transform \n";
}

static void surface_set_buffer_scale(wl_client *client, wl_resource *resource, int32_t scale)
{
    std::cout << "[LumaCompositor] surface_set_buffer_scale \n";
}

static void surface_damage_buffer(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] surface_damage_buffer \n";
}

static void surface_offset(wl_client *client,wl_resource *resource,int32_t x,int32_t y)
{
    std::cout << "[LumaCompositor] surface_offset \n";
}

static const struct wl_surface_interface surface_impl = {
    .destroy = surface_destroy,
    .attach = surface_attach,
    .damage = surface_damage,
    .frame = surface_frame,
    .set_opaque_region = surface_set_opaque_region,
    .set_input_region = surface_set_input_region,
    .commit = surface_commit,
    .set_buffer_transform = surface_set_buffer_transform,
    .set_buffer_scale = surface_set_buffer_scale,
    .damage_buffer = surface_damage_buffer,
    .offset = surface_offset
};

static void surface_resource_destroy(struct wl_resource *resource)
{
    std::cout << "[LumaCompositor] surface_resource_destroy\n";

    auto *surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    std::cerr << "[DEBUG] Deleting my_surface " << surf << " (surface=" << resource << ")\n";
    if (!surf) 
    { 
        std::cerr << "[DEBUG] surf NULL in xdg handler\n";
        return; 
    }

    // If xdg/toplevel still exist, detach their user_data so they don't delete same wrapper later
    if (surf->xdg_surface_res)
    {
        wl_resource_set_user_data(surf->xdg_surface_res, nullptr);
    }

    if (surf->toplevel_res)
    {
        wl_resource_set_user_data(surf->toplevel_res, nullptr);
    }

    if (surf->buffer_res)
    {
        wl_resource_set_user_data(surf->buffer_res, nullptr);
    }

    delete surf;

    // clear surface user_data (defensive)
    wl_resource_set_user_data(resource, nullptr);
}

// --------------------------- xdg_surface ---------------------------

static void destroy_xdg_surface_resource(struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] destroy_xdg_surface_resource\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf)
    {
        return;
    }
    surf->xdg_surface_res = nullptr;
    // do not delete surf here — surface_resource_destroy owns lifecycle
}

static void destroy_xdg_toplevel_resource(struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] destroy_xdg_toplevel_resource\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf) return;
    surf->toplevel_res = nullptr;
    // do not delete surf here — surface_resource_destroy owns lifecycle

    wl_resource_set_user_data(resource, nullptr);
}

// ------------------ xdg_toplevel ------------------
static void xdg_toplevel_destroy(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_toplevel_destroy\n";
}

static void xdg_toplevel_set_parent(struct wl_client*, struct wl_resource*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_parent\n";
}

static void xdg_toplevel_set_title(struct wl_client*, struct wl_resource*, const char*)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_title\n";
}

static void xdg_toplevel_set_app_id(struct wl_client*, struct wl_resource*, const char*) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_app_id\n";
}

// static void xdg_toplevel_show_window_menu(struct wl_client*, struct wl_resource*, struct wl_resource*, int32_t, int32_t) {}

static void xdg_toplevel_show_window_menu(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_resource *seat,
				 uint32_t serial,
				 int32_t x,
				 int32_t y) 
{
    std::cout << "[LumaCompositor] Created xdg_toplevel_show_window_menu window\n";
}

static void xdg_toplevel_move(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t) {}
static void xdg_toplevel_resize(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t, uint32_t) {}
static void xdg_toplevel_set_max_size(struct wl_client*, struct wl_resource*, int32_t, int32_t) {}
static void xdg_toplevel_set_min_size(struct wl_client*, struct wl_resource*, int32_t, int32_t) {}
static void xdg_toplevel_set_maximized(struct wl_client*, struct wl_resource*) {}
static void xdg_toplevel_unset_maximized(struct wl_client*, struct wl_resource*) {}
static void xdg_toplevel_set_fullscreen(struct wl_client*, struct wl_resource*, struct wl_resource*) {}
static void xdg_toplevel_unset_fullscreen(struct wl_client*, struct wl_resource*) {}
static void xdg_toplevel_set_minimized(struct wl_client*, struct wl_resource*) {}

static const struct xdg_toplevel_interface xdg_toplevel_impl = {
    .destroy = xdg_toplevel_destroy,
    .set_parent = xdg_toplevel_set_parent,
    .set_title = xdg_toplevel_set_title,
    .set_app_id = xdg_toplevel_set_app_id,
    .show_window_menu = xdg_toplevel_show_window_menu,
    .move = xdg_toplevel_move,
    .resize = xdg_toplevel_resize,
    .set_max_size = xdg_toplevel_set_max_size,
    .set_min_size = xdg_toplevel_set_min_size,
    .set_maximized = xdg_toplevel_set_maximized,
    .unset_maximized = xdg_toplevel_unset_maximized,
    .set_fullscreen = xdg_toplevel_set_fullscreen,
    .unset_fullscreen = xdg_toplevel_unset_fullscreen,
    .set_minimized = xdg_toplevel_set_minimized
};



// ------------------ xdg_surface ------------------
static void xdg_surface_destroy(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_surface_destroy...\n";

}

// static void destroy_xdg_toplevel_resource(wl_resource* resource)
// {
//     std::cout << "[LumaCompositor] destroy_xdg_toplevel_resource...\n";

//     my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
//     if (!surf) return;

//     surf->toplevel_res = nullptr;
//     if (!surf->xdg_surface_res) {
//         delete surf;
//     }
// }

static void xdg_surface_get_toplevel(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] xdg_surface_get_toplevel...\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf)
    {
        std::cerr << "[LumaCompositor] ERROR: xdg_surface has no wrapper user_data\n";
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), xdg_toplevel_interface.version);
    wl_resource* toplevel = wl_resource_create(client, &xdg_toplevel_interface, ver, id);
    if (!toplevel) {
        wl_client_post_no_memory(client);
        return;
    }

    // attach the same surface-wrapper (or a new wrapper for toplevel) as user_data
    wl_resource_set_implementation(toplevel, &xdg_toplevel_impl, surf, destroy_xdg_toplevel_resource);

    // wl_resource_set_implementation(
    // toplevel,
    // &xdg_toplevel_impl,
    // surf,
    // [](wl_resource* res){
    //     my_surface* s = static_cast<my_surface*>(wl_resource_get_user_data(res));
    //     s->toplevel_res = nullptr;
    //     if (!s->xdg_surface_res) delete s; // delete only if no xdg_surface
    // }
    // );

    surf->toplevel_res = toplevel;


    uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    xdg_surface_send_configure(resource, serial);

    std::cout << "[LumaCompositor] End xdg_surface_get_toplevel...\n";
}

static void xdg_surface_get_popup(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_surface_get_popup...\n";
}

static void xdg_surface_set_window_geometry(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] xdg_surface_set_window_geometry...\n";
}

static void xdg_surface_ack_configure(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_surface_ack_configure...\n";
}

static const struct xdg_surface_interface xdg_surface_impl = {
    .destroy = xdg_surface_destroy,
    .get_toplevel = xdg_surface_get_toplevel,
    .get_popup = xdg_surface_get_popup,
    .set_window_geometry = xdg_surface_set_window_geometry,
    .ack_configure = xdg_surface_ack_configure
};


// ------------------ xdg_wm_base ------------------
static void xdg_wm_base_destroy(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_wm_base_destroy...\n";

}






// ------------------ xdg_positioner ------------------
static void xdg_positioner_destroy(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_positioner_destroy\n";
}
static void xdg_positioner_set_size(struct wl_client*, struct wl_resource*, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_size\n";
}
static void xdg_positioner_set_anchor_rect(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_anchor_rect\n";
}
static void xdg_positioner_set_anchor(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_anchor\n";
}
static void xdg_positioner_set_gravity(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_gravity\n";
}
static void xdg_positioner_set_constraint_adjustment(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_constraint_adjustment\n";
}
static void xdg_positioner_set_offset(struct wl_client*, struct wl_resource*, int32_t, int32_t) 
{
    std::cout << "[LumaCompositor] xdg_positioner_set_offset\n";
}
static void xdg_positioner_set_reactive(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_reactive\n";
}
static void xdg_positioner_set_parent_size(struct wl_client*, struct wl_resource*, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_parent_size\n";
}
static void xdg_positioner_set_parent_configure(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_positioner_set_parent_configure\n";
}

static const struct xdg_positioner_interface xdg_positioner_impl = {
    .destroy = xdg_positioner_destroy,
    .set_size = xdg_positioner_set_size,
    .set_anchor_rect = xdg_positioner_set_anchor_rect,
    .set_anchor = xdg_positioner_set_anchor,
    .set_gravity = xdg_positioner_set_gravity,
    .set_constraint_adjustment = xdg_positioner_set_constraint_adjustment,
    .set_offset = xdg_positioner_set_offset,
    .set_reactive = xdg_positioner_set_reactive,
    .set_parent_size = xdg_positioner_set_parent_size,
    .set_parent_configure = xdg_positioner_set_parent_configure
};


static void xdg_wm_base_create_positioner(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] xdg_wm_base_create_positioner...\n";
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), xdg_surface_interface.version);

    wl_resource* pos = wl_resource_create(client, &xdg_positioner_interface, ver, id);
    wl_resource_set_implementation(pos, &xdg_positioner_impl, nullptr, nullptr);
}

static void xdg_surface_resource_destroy(struct wl_resource* resource) {
    // resource user_data should be a pointer to the xdg-surface wrapper (if you allocate one)
    // or if you used the my_surface as user_data, do nothing here if it's managed by wl_surface destroy.

    if (!resource)
    {
        std::cout << "[LumaCompositor] xdg_surface_resource_destroy return !resource...\n";
        return;
    }

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf)
    {
        std::cout << "[LumaCompositor] xdg_surface_resource_destroy return !surf...\n";
        return;
    }

    surf->xdg_surface_res = nullptr;
    if (!surf->toplevel_res)
    {
        delete surf; // delete only if no toplevel
    }

    // clear user_data so no future callbacks try to use it
    wl_resource_set_user_data(resource, nullptr);
}

static void xdg_wm_base_get_xdg_surface(struct wl_client* client, struct wl_resource* resource,
                                        uint32_t id, struct wl_resource* surface)
{
    std::cout << "[LumaCompositor] xdg_wm_base_get_xdg_surface...\n";

    // surface_res is the existing wl_surface resource. Retrieve our wrapper:
    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface));
    if (!surf) {
        std::cerr << "[LumaCompositor] ERROR: passed wl_surface has no user_data\n";
        wl_client_post_no_memory(client);
        return;
    }

    // use the version of the xdg_surface resource to create the child
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), xdg_surface_interface.version);

    wl_resource* xdg_surf = wl_resource_create(client, &xdg_surface_interface, ver, id);
    if (!xdg_surf)
    {
        wl_client_post_no_memory(client);
        return;
    }

    // It's common to either:
    //  - attach the xdg_surface wrapper as user_data, or
    //  - attach the underlying my_surface (if you want to map xdg->wl surface)
    // Here we attach the wl_surface wrapper pointer so you can find it later:
    wl_resource_set_implementation(xdg_surf, &xdg_surface_impl, surf, xdg_surface_resource_destroy);


    // remember xdg resource if needed
    surf->xdg_surface_res = xdg_surf;
}

static void xdg_wm_base_pong(struct wl_client*, struct wl_resource*, uint32_t)
{
    std::cout << "[LumaCompositor] xdg_wm_base_pong...\n";

}

static const struct xdg_wm_base_interface xdg_wm_base_impl = {
    .destroy = xdg_wm_base_destroy,
    .create_positioner = xdg_wm_base_create_positioner,
    .get_xdg_surface = xdg_wm_base_get_xdg_surface,
    .pong = xdg_wm_base_pong
};

// --------------------------- wl_compositor ---------------------------

static void compositor_create_surface(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] compositor_create_surface\n";

    // negotiate version: use the version requested by client when created (resource passed is the compositor global resource)
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), wl_surface_interface.version);
    wl_resource* surface_res = wl_resource_create(client, &wl_surface_interface, ver, id);
    if (!surface_res)
    {
        wl_client_post_no_memory(client);
        return;
    }

    my_surface* surf = new my_surface();
    surf->resource = surface_res;

    wl_resource_set_implementation(surface_res, &surface_impl, surf, surface_resource_destroy);
    std::cout << "[LumaCompositor] Client created surface (resource=" << surface_res << ")\n";
}

static void compositor_create_region(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] compositor_create_region\n";
    wl_resource_create(client, &wl_region_interface, 1, id);
    std::cout << "[LumaCompositor] End compositor_create_region\n";
}

static const struct wl_compositor_interface compositor_impl = {
    .create_surface = compositor_create_surface,
    .create_region = compositor_create_region
};

static void bind_compositor(struct wl_client* client, void* data, uint32_t version, uint32_t id) 
{
    std::cout << "[LumaCompositor] bind_compositor\n";

    uint32_t ver = std::min<uint32_t>(version, wl_compositor_interface.version);
    wl_resource* res = wl_resource_create(client, &wl_compositor_interface, ver, id);
    wl_resource_set_implementation(res, &compositor_impl, nullptr, nullptr);
}

// --------------------------- wl_output (minimal) ---------------------------
static void wl_output_handle_release(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_output_interface
wl_output_implementation = {
    .release = wl_output_handle_release,
};


static void wl_output_handle_resource_destroy(struct wl_resource* resource)
{
    if (!resource)
    {
        fprintf(stderr, "[luma] destroy called with null resource\n");
        return;
    }

    auto* client_output = static_cast<my_output*>(wl_resource_get_user_data(resource));
    if (!client_output)
    {
        fprintf(stderr, "[luma] destroy: user_data == NULL\n");

        return;
    }

    if (client_output->state) {
        remove_from_list(client_output->state->client_outputs, client_output);
    }

    delete client_output;
}


static void wl_output_handle_bind(struct wl_client* client, void* data,
                                  uint32_t version, uint32_t id)
{
    std::cout << "[LumaCompositor] wl_output_handle_bind...\n";

    if (!data)
    {
        std::cout << "[LumaCompositor]] ERROR: bind called with null state"<<std::endl;
        return;
    }

    auto* state = static_cast<LumaCompositor*>(data);

    my_output* client_output = new my_output();
    uint32_t ver = std::min<uint32_t>(version, wl_output_interface.version);
    wl_resource* resource = wl_resource_create(client, &wl_output_interface, ver, id);
    client_output->state = state;

    if (!resource) {
        std::cout << "[LumaCompositor]] ERROR: wl_resource_create returned NULL"<<std::endl;
        delete client_output;
        return;
    }

    wl_resource_set_implementation(
        resource,
        &wl_output_implementation,
        client_output,
        wl_output_handle_resource_destroy
    );

    client_output->resource = resource;


    add_to_list(state->client_outputs, client_output);

    // wl_output_send_geometry(resource, 0, 0, 1920, 1080,
    // WL_OUTPUT_SUBPIXEL_UNKNOWN, "Foobar, Inc",
    // "Fancy Monitor 9001 4K HD 120 FPS Noscope",
    // WL_OUTPUT_TRANSFORM_NORMAL);

    // Send required events depending on bound version.
    int res_ver = wl_resource_get_version(resource);

    // Use safe non-null strings
    const char* make = "LumaCo";
    const char* model = "VirtualDisplay-1";
    const char* name = "luma-0";
    const char* description = "Luma virtual output";

    // geometry: since v1
    // x,y and physical mm: provide useful defaults (0 is allowed but not ideal)
    int x = 0, y = 0;
    int physical_w_mm = 310; // set a realistic value or 0
    int physical_h_mm = 170;


    wl_output_send_geometry(resource,
                        x, y,
                        physical_w_mm, physical_h_mm,
                        WL_OUTPUT_SUBPIXEL_NONE,
                        make, model,
                        WL_OUTPUT_TRANSFORM_NORMAL);

    // mode: since v1
    int mode_flags = WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED;
    wl_output_send_mode(resource, mode_flags,
                        client_output->width, client_output->height,
                        client_output->refresh_rate);

    // scale: since v3 (some implementations accept v2 or v3 - check your generated headers)
    if (res_ver >= 3) {
        wl_output_send_scale(resource, client_output->scale);
    }

    // done: since v2 (clients rely on done to know the values are complete)
    if (res_ver >= 2) {
        wl_output_send_done(resource);
    }

    // name/description: since v4
    if (res_ver >= 4) {
        wl_output_send_name(resource, name);
        wl_output_send_description(resource, description);
    }

    // --- TODO: Send geometry, mode, scale, etc. ---
    // wl_output_send_geometry(resource, ...);
    // wl_output_send_mode(resource, ...);
    // wl_output_send_done(resource);

    std::cout << "[LumaCompositor] END of wl_output_handle_bind...\n";
}


// --------------------------- client created listener ---------------------------
static void handle_new_client(struct wl_listener *listener, void *data)
{
    auto *client = static_cast<wl_client*>(data);
    pid_t pid;
    uid_t uid;
    gid_t gid;

    wl_client_get_credentials(client, &pid, &uid, &gid);
    printf("[Wayland] New client connected (pid=%d, uid=%d)\n", pid, uid);
}

static void bind_shm(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
    std::cout << "[LumaCompositor] bind_shm\n";

    uint32_t ver = std::min<uint32_t>(version, wl_shm_interface.version);
    wl_resource* res = wl_resource_create(client, &wl_shm_interface, ver, id);

    if (!res) 
    {
        wl_client_post_no_memory(client);
        return;
    }

    // advertise a common format
    if (ver >= WL_SHM_FORMAT_ARGB8888) { // WL_SHM_FORMAT_* are enums; check header if needed
        wl_shm_send_format(res, WL_SHM_FORMAT_ARGB8888);
    } else {
        // still send something if available (older headers unlikely)
        wl_shm_send_format(res, WL_SHM_FORMAT_ARGB8888);
    }

    wl_resource_set_implementation(res, &shm_impl, nullptr, nullptr);
}



static void bind_xdg_wm_base(struct wl_client* client, void*, uint32_t version, uint32_t id)
{
    std::cout << "[LumaCompositor] bind_xdg_wm_base...\n";

    wl_resource* res = wl_resource_create(client, &xdg_wm_base_interface, version, id);
    wl_resource_set_implementation(res, &xdg_wm_base_impl, nullptr, nullptr);



    std::cout << "[LumaCompositor] Sending initial xdg_wm_base ping...\n";
    uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    xdg_wm_base_send_ping(res, serial);

    std::cout << "[LumaCompositor] End bind_xdg_wm_base...\n";

}



void setup_wayland_display(wl_display* display) 
{
    std::cout << "[LumaCompositor] setup_wayland_display...\n";

    static wl_listener client_listener;
    client_listener.notify = handle_new_client;
    wl_display_add_client_created_listener(display, &client_listener);
}


// ------------------ Init & Run ------------------

void luma_init(LumaCompositor* comp)
{
    comp->display = wl_display_create();
    if (!comp->display) {
        std::cerr << "[LumaCompositor] Failed to create display\n";
        exit(1);
    }

    std::string displayName = "luma-0";

    if (wl_display_add_socket(comp->display, displayName.c_str()))
    {
        std::cerr << "[LumaCompositor] Socket luma-0 in use, trying luma-1...\n";

        displayName = "luma-1";

        if (wl_display_add_socket(comp->display, displayName.c_str())) {
            std::cerr << "[LumaCompositor] Failed to add any Wayland socket.\n";
            return;
        }
    }

    setenv("WAYLAND_DISPLAY", displayName.c_str(), 1);
    std::cout << "[LumaCompositor] Using Wayland socket: "<<displayName<<std::endl;

    // const char* socket = wl_display_add_socket_auto(comp->display);
    // if (!socket) {
    //     std::cerr << "[LumaCompositor] Failed to add Wayland socket\n";
    //     exit(1);
    // }

    // std::cout << "[LumaCompositor] Using Wayland socket: " << socket << "\n";

    comp->loop = wl_display_get_event_loop(comp->display);


    comp->output_global = wl_global_create(comp->display,
                                            &wl_output_interface,
                                            wl_output_interface.version,
                                            comp,
                                            wl_output_handle_bind);

    comp->compositor_global = wl_global_create(comp->display,
                                               &wl_compositor_interface,
                                               4,                           // version from wayland.xml
                                               nullptr, bind_compositor);
    comp->shm_global = wl_global_create(comp->display,
                                        &wl_shm_interface,
                                        1, nullptr, bind_shm);
    comp->xdg_wm_base_global = wl_global_create(comp->display,
                                                &xdg_wm_base_interface,
                                                4,                          // version from your xdg-shell.xml
                                                nullptr, bind_xdg_wm_base);

    setup_wayland_display(comp->display);

    std::cout << "[LumaCompositor] Wayland display initialized\n";
}

void luma_run(LumaCompositor* comp)
{
    std::cout << "[LumaCompositor] Running Wayland event loop...\n";
    wl_display_run(comp->display);
}