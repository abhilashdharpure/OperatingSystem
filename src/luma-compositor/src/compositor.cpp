#include "compositor.h"
#include "compositor_helper.h"

#include <iostream>
#include <wayland-server.h>

// #include "build/generated/xdg-shell-protocol.h"
// #include "build/generated/wayland-protocol.h"
#include "xdg-shell-protocol.h"
#include "wayland-protocol.h"
#include <unistd.h>   // for close()
#include <sys/mman.h>
#include <wayland-server-core.h>
#include <cstring>

#include <SDL2/SDL.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <xkbcommon/xkbcommon.h>
#include <chrono>

CompositorInput compositorInput;
SDL_Window* window;
SDL_Texture* texture;
SDL_Renderer* renderer;
static std::vector<uint32_t> comp_framebuffer; // stored as ARGB8888 (32-bit words)
static std::mutex comp_fb_mutex;
static std::atomic<bool> sdl_thread_running{false};
static std::thread sdl_thread;

using namespace std;

// --- Simple linked list utilities using std::list ---
static void add_to_list(std::list<my_output*>& lst, my_output* item)
{
    lst.push_back(item);
}

static void remove_from_list(std::list<my_output*>& lst, my_output* item)
{
    lst.remove(item);
}

uint32_t get_current_time_ms()
{
    auto now = chrono::system_clock::now();

    // Convert the current time to time since epoch
    auto duration = now.time_since_epoch();

    // Convert duration to milliseconds
    auto milliseconds
        = chrono::duration_cast<chrono::milliseconds>(
              duration)
              .count();

    // Print the result
    // std::cout << "Current time in milliseconds is: "<< milliseconds << std::endl;

    return milliseconds;
}

void destroy_keymap_fd(struct wl_listener* listener, void* data)
{
    keymap_fd_holder* holder = wl_container_of(listener, holder, listener);
    close(holder->fd);
    delete holder;
}

void send_keymap_to_client(wl_resource* keyboard_res)
{
    std::cout << "[LumaCompositor] send_keymap_to_client" << std::endl;
    struct xkb_context* ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_rule_names names = { "evdev", "pc105", "us", "", "" };
    struct xkb_keymap* keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    const char* keymap_str = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
    size_t size = strlen(keymap_str) + 1;

    int fd = memfd_create("keymap", MFD_CLOEXEC);
    ftruncate(fd, size);
    void* map = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(map, keymap_str, size);
    munmap(map, size);

    wl_keyboard_send_keymap(keyboard_res, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);

    // defer close
    keymap_fd_holder* holder = new keymap_fd_holder;
    holder->fd = fd;
    holder->listener.notify = destroy_keymap_fd;
    wl_resource_add_destroy_listener(keyboard_res, &holder->listener);

    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
}

void fb_flush(LumaCompositor *comp)
{
    SDL_UpdateTexture(texture, nullptr, comp_framebuffer.data(), comp->output_width * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void UpdateFrameBuffer(LumaCompositor* compositor, my_surface* surf)
{
    std::cout<<"[LumaCompositor] UpdateFrameBuffer "<<std::endl;

    if (!surf) return;
    if (!surf->buffer_res) return; // use current_buffer_res

    auto* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(surf->buffer_res));
    if (!buf || !buf->data) return;

    // geometry as reported by xdg_surface_set_window_geometry() or surface fields
    int dst_x = surf->x;
    int dst_y = surf->y;
    int geo_w = surf->width;
    int geo_h = surf->height;

    // Source buffer width/height - buf->width/buf->height
    // Clip to framebuffer boundaries
    if (dst_x >= compositor->output_width || dst_y >= compositor->output_height)
        return;

    int copy_w = std::min(geo_w, compositor->output_width - dst_x);
    int copy_h = std::min(geo_h, compositor->output_height - dst_y);
    if (copy_w <= 0 || copy_h <= 0) return;

    uint8_t* src8 = reinterpret_cast<uint8_t*>(buf->data);
    uint8_t* dst8 = reinterpret_cast<uint8_t*>(comp_framebuffer.data());

    // buf->stride is bytes per source row
    for (int row = 0; row < copy_h; ++row) {
        uint8_t* srow = src8 + row * buf->stride; // start of row inside the buffer
        uint8_t* drow = dst8 + (dst_y + row) * compositor->output_width * 4 + dst_x * 4;
        memcpy(drow, srow, copy_w * 4); // assume ARGB32 (4 bytes/pix)
    }
    // DO NOT send wl_buffer_send_release here - compositor still needs this buffer.

    // if(surf != nullptr)
    // {
    //     if(surf->buffer_res == nullptr)
    //     {
    //         return;
    //     }
    //     auto* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(surf->buffer_res));
    //     if (buf && buf->data)
    //     {
    //         // geometry as reported by xdg_surface_set_window_geometry()
    //         int geo_x = surf->x;
    //         int geo_y = surf->y;
    //         int geo_w = surf->width;
    //         int geo_h = surf->height;

    //         int dst_x = surf->x;
    //         int dst_y = surf->y;

    //         // std::cout << "[LumaCompositor] surface_commit, "
    //         //   << "buf_w=" << buf->width << ", buf_h=" << buf->height
    //         //   << ", geo_x=" << geo_x << ", geo_y=" << geo_y
    //         //   << ", geo_w=" << geo_w << ", geo_h=" << geo_h
    //         //   << ", dst_x=" << dst_x << ", dst_y=" << dst_y << std::endl;

    //         // std::cout << "[LumaCompositor] surface_commit, x = "<<dst_x <<", y = "<<dst_y<<", width = "<<copy_w<<", height = "<<copy_h<<std::endl;;

    //         uint8_t* src8 = reinterpret_cast<uint8_t*>(buf->data);
    //         uint8_t* dst8 = reinterpret_cast<uint8_t*>(comp_framebuffer.data());

    //         // Clip to framebuffer size
    //         int copy_w = std::min(geo_w, compositor->output_width - dst_x);
    //         int copy_h = std::min(geo_h, compositor->output_height - dst_y);
            
    //         for (int y = 0; y < copy_h; ++y) {
    //             int src_y = geo_y + y;
    //             uint8_t* srow = src8 + src_y * buf->stride + geo_x * 4;
    //             uint8_t* drow = dst8 + (dst_y + y) * compositor->output_width * 4 + dst_x * 4;
    //             memcpy(drow, srow, copy_w * 4);
    //         }
            

    //         fb_flush(compositor);

    //         if (surf->buffer_res)
    //         {
    //             wl_buffer_send_release(surf->buffer_res);
    //             surf->buffer_res = nullptr;
    //         }
    //     }
    // }
}

void compositor_repaint(LumaCompositor* comp)
{
    std::cout << "[LumaCompositor] compositor_repaint"<<std::endl;

    if (!comp->needs_repaint)
    {
        return;
    }

    comp->needs_repaint = false;

    std::lock_guard<std::mutex> lock(comp_fb_mutex);

    int W = comp->output_width;
    int H = comp->output_height;

    // 1. Clear framebuffer (ARGB)
    memset(comp_framebuffer.data(), 0x00, W * H * 4);

    // 2. Composite all surfaces in stacking order (back → front)
    for (my_surface* surf : comp->surfaces)   // comp->surfaces is ordered
    {
        if (!surf->mapped)   // mapped only after first commit
        {
            continue;
        }

        UpdateFrameBuffer(comp, surf);
    }

    // Send wl_surface.frame done callbacks
    uint32_t time_ms = get_current_time_ms();
    for (my_surface* surf : comp->surfaces)
    {
        if (surf->pending_frame_callback)
        {
            wl_callback_send_done(surf->pending_frame_callback, time_ms);
            wl_resource_destroy(surf->pending_frame_callback);
            surf->pending_frame_callback = nullptr;
        }
    }

    // Push to output backend (your SDL renderer or kernel fb)
    fb_flush(comp);
}

static const struct wl_keyboard_interface keyboard_impl = {
    .release = [](struct wl_client* client, struct wl_resource* resource) {
        wl_resource_destroy(resource);
    },
};

wl_resource* seat_get_pointer(LumaSeat* seat, wl_client* client)
{
    std::cout << "[LumaCompositor] seat_get_pointer\n";

    wl_resource* resource;
    wl_list_for_each(resource, &seat->pointers, link) {
        if (wl_resource_get_client(resource) == client)
            return resource;
    }
    return nullptr;
}

static void keyboard_resource_destroy(struct wl_resource *resource)
{
    std::cout << "[LumaCompositor] keyboard_resource_destroy\n";

    // nothing stored as user_data currently
    wl_resource_set_user_data(resource, nullptr);
}

void safe_send_keyboard_enter(LumaCompositor* compositor,
                              struct wl_resource* focused_surface,
                              struct wl_display* display)
{
    std::cout << "[LumaCompositor] safe_send_keyboard_enter\n";

    if (!compositor->keyboard_resource || !focused_surface || !display)
    {
        return;
    }

    if (!focused_surface || !display)
    {
        return;
    }

    wl_client* focused_client = wl_resource_get_client(focused_surface);
    if (!focused_client)
    {
        return;
    }

    compositorInput.SendKeyboardEnterEvent(compositor, focused_surface);
}

static void seat_get_keyboard(struct wl_client *client, struct wl_resource *seat_res, uint32_t id)
{
    std::cout << "[LumaCompositor] seat_get_keyboard\n";

    LumaSeat* seat = (LumaSeat*)wl_resource_get_user_data(seat_res);
    wl_resource* keyboard_res = wl_resource_create(client, &wl_keyboard_interface, 1, id);

    wl_resource_set_implementation(keyboard_res, &keyboard_impl, nullptr, keyboard_resource_destroy);
    wl_list_insert(&seat->keyboards, wl_resource_get_link(keyboard_res));

    send_keymap_to_client(keyboard_res);
    compositorInput.AddKeyboardEvent(keyboard_res);

    LumaCompositor* compositor = seat->compositor;
    compositor->keyboard_resource = keyboard_res;
}

static void wl_pointer_interface_set_cursor(struct wl_client* client, struct wl_resource* resource, uint32_t serial,
                     struct wl_resource* surface, int32_t hotspot_x, int32_t hotspot_y)
{
    // std::cout << "[LumaCompositor] wl_pointer_interface_set_cursor, hotspot_x = "<<hotspot_x<<", hotspot_y = "<<hotspot_y<<std::endl;

}


static void wl_pointer_interface_release(struct wl_client* client, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] wl_pointer_interface_release\n";
    wl_resource_destroy(resource);
}

static const struct wl_pointer_interface pointer_impl = {
    .set_cursor = wl_pointer_interface_set_cursor,
    .release = wl_pointer_interface_release,
};

static void seat_get_pointer(struct wl_client *client, struct wl_resource *seat_res, uint32_t id)
{
    std::cout << "[LumaCompositor] seat_get_pointer\n";
    LumaSeat* seat = (LumaSeat*)wl_resource_get_user_data(seat_res);
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(seat_res), wl_pointer_interface.version);

    wl_resource* pointer_res = wl_resource_create(client, &wl_pointer_interface, ver, id);
    wl_resource_set_implementation(pointer_res, &pointer_impl, seat, nullptr);
    // g_pointers.push_back(pointer_res);
    seat->pointer_res = pointer_res;
    compositorInput.AddKeyMouseEvent(pointer_res);
}

static void seat_get_touch(struct wl_client* client,
                           struct wl_resource* seat_res,
                           uint32_t id)
{
    std::cout << "[LumaCompositor] seat_get_touch\n";
}

static void seat_get_release(struct wl_client *client,
			struct wl_resource *resource)
{
    std::cout << "[LumaCompositor] seat_get_release\n";
}

static const struct wl_seat_interface seat_interface = {
    .get_pointer = seat_get_pointer,
    .get_keyboard = seat_get_keyboard,
    .get_touch = seat_get_touch,
    .release = seat_get_release,
};

static void seat_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
    std::cout << "[LumaCompositor] seat_bind\n";

    LumaSeat* seat = (LumaSeat*)data;

    uint32_t ver = std::min<uint32_t>(version, wl_seat_interface.version);
    wl_resource* resource = wl_resource_create(client, &wl_seat_interface, ver, id);
    wl_resource_set_implementation(resource, &seat_interface, seat, nullptr);

    wl_seat_send_capabilities(resource,
        WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);

    wl_seat_send_name(resource, "luma-seat");
}

static void buffer_resource_destroy(struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] buffer_resource_destroy\n";

    shm_buffer* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(resource));
    if (!buf)
    {
        return;
    }

    if (buf->owner_surface)
    {
        buf->owner_surface->buffer_res = nullptr;
    }

    if (buf->data && buf->size)
    {
        munmap(buf->data, buf->size);
    }

    delete buf;

    wl_resource_set_user_data(resource, nullptr);
}

static void buffer_destroy_request(struct wl_client* client, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] buffer_destroy_request\n";

    (void)client;
    // client requested to destroy buffer resource - server can ignore or destroy
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface buffer_impl = {
    .destroy = buffer_destroy_request
};

static void shm_pool_resize(struct wl_client* client, struct wl_resource* resource, int32_t new_size)
{
    shm_pool_data* pool = (shm_pool_data*) wl_resource_get_user_data(resource);
    if (!pool) return;

    if (pool->data)
        munmap(pool->data, pool->size);

    pool->size = new_size;
    pool->data = (uint8_t*) mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, pool->fd, 0);
    if (pool->data == MAP_FAILED) {
        wl_resource_post_no_memory(resource);
        pool->data = nullptr;
        return;
    }

    std::cout << "[LumaCompositor] wl_shm_pool_resize: remapped new size " << new_size << std::endl;
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
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(pool_res), wl_buffer_interface.version);
    wl_resource *buf_res = wl_resource_create(client, &wl_buffer_interface, ver, id);
    if (!buf_res)
    {
        wl_client_post_no_memory(client);
        return;
    }

    // retrieve pool pointer
    auto *pool = static_cast<shm_pool_data*>(wl_resource_get_user_data(pool_res));
    if (!pool || !pool->data)
    {
        wl_client_post_no_memory(client);
        wl_resource_destroy(buf_res);
        return;
    }

    // basic bounds check
    size_t needed = (size_t)stride * (size_t)height;
    if ((size_t)offset + needed > pool->size)
    {
        std::cerr << "[LumaCompositor] shm_pool_create_buffer: out-of-bounds\n";
        wl_client_post_no_memory(client);
        wl_resource_post_error(pool_res, WL_SHM_ERROR_INVALID_STRIDE, "buffer outside pool");
        wl_resource_destroy(buf_res);
        return;
    }

    shm_buffer* buf = new shm_buffer;
    buf->resource = buf_res;
    buf->data = static_cast<uint8_t*>(pool->data) + offset;
    buf->size = needed;
    buf->width = width;
    buf->height = height;
    buf->stride = stride;
    buf->format = format;

    wl_resource_set_implementation(buf_res, &buffer_impl, buf, buffer_resource_destroy);

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
    // std::cout << "[LumaCompositor] surface_attach\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));

    if (!surf)
    {
        std::cout << "[LumaCompositor] ERROR:  surface_attach,surf is NULL\n";

        return;
    }

    shm_buffer* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(buffer));
    if (buf)
    {
        buf->owner_surface = surf;
    }
    surf->buffer_res = buffer; // just track it

}

// static void surface_commit(wl_client* client, wl_resource* surface_res)
// {
//     std::cout << "[LumaCompositor] surface_commit\n";
//     my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));
//     if (!surf) return;

//     LumaCompositor* comp = surf->compositor;
//     if (!comp) return;

//     // Surface becomes visible on first commit
//     surf->mapped = true;

//     // Nothing to render yet if no buffer is attached
//     if (!surf->buffer_res) {
//         // If client requested frame callback → send it immediately
//         if (surf->pending_frame_callback) {
//             uint32_t time = get_current_time_ms();
//             wl_callback_send_done(surf->pending_frame_callback, time);
//             wl_resource_destroy(surf->pending_frame_callback);
//             surf->pending_frame_callback = nullptr;
//         }
//         return;
//     }

//     //----------------------------------------------------------
//     // DO NOT BLIT / DRAW PIXELS HERE!
//     // Just mark that compositor needs a repaint.
//     //----------------------------------------------------------
//     comp->needs_repaint = true;

//     //----------------------------------------------------------
//     // Release old buffer now that we have committed it
//     //----------------------------------------------------------
//     wl_buffer_send_release(surf->buffer_res);
//     surf->buffer_res = nullptr;

//     //----------------------------------------------------------
//     // Do NOT send frame done here if you are repainting later.
//     compositor_repaint(comp); //will send frame done.
//     //----------------------------------------------------------
// }


static void surface_commit(wl_client* client, wl_resource* surface_res)
{
    // std::cout << "[LumaCompositor] surface_commit\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));
    if (!surf)
    {
        return;
    }
    LumaCompositor* compositor = surf->compositor;
    surf->mapped = true;
    compositor->needs_repaint = true;

    // if no buffer attached → nothing to show, but still respond to callbacks
    if (surf->pending_frame_callback && surf->buffer_res)
    {
        // std::lock_guard<std::mutex> lk(comp_fb_mutex);
        // UpdateFrameBuffer(compositor, surf);

        auto* buf = static_cast<shm_buffer*>(wl_resource_get_user_data(surf->buffer_res));
        if (buf && buf->data)
        {
            std::lock_guard<std::mutex> lk(comp_fb_mutex);

            // geometry as reported by xdg_surface_set_window_geometry()
            int geo_x = surf->x;
            int geo_y = surf->y;
            int geo_w = surf->width;
            int geo_h = surf->height;

            int dst_x = surf->x;
            int dst_y = surf->y;

            // std::cout << "[LumaCompositor] surface_commit, "
            //   << "buf_w=" << buf->width << ", buf_h=" << buf->height
            //   << ", geo_x=" << geo_x << ", geo_y=" << geo_y
            //   << ", geo_w=" << geo_w << ", geo_h=" << geo_h
            //   << ", dst_x=" << dst_x << ", dst_y=" << dst_y << std::endl;

            // std::cout << "[LumaCompositor] surface_commit, x = "<<dst_x <<", y = "<<dst_y<<", width = "<<copy_w<<", height = "<<copy_h<<std::endl;;

            uint8_t* src8 = reinterpret_cast<uint8_t*>(buf->data);
            uint8_t* dst8 = reinterpret_cast<uint8_t*>(comp_framebuffer.data());

            // Clip to framebuffer size
            int copy_w = std::min(geo_w, compositor->output_width - dst_x);
            int copy_h = std::min(geo_h, compositor->output_height - dst_y);
            
            for (int y = 0; y < copy_h; ++y) {
                int src_y = geo_y + y;
                uint8_t* srow = src8 + src_y * buf->stride + geo_x * 4;
                uint8_t* drow = dst8 + (dst_y + y) * compositor->output_width * 4 + dst_x * 4;
                memcpy(drow, srow, copy_w * 4);
            }
            

            fb_flush(compositor);

            if (surf->buffer_res)
            {
                wl_buffer_send_release(surf->buffer_res);
                surf->buffer_res = nullptr;
            }
        }

        // Send frame done
        wl_client* c = wl_resource_get_client(surf->pending_frame_callback);
        wl_display* d = wl_client_get_display(c);
        uint32_t serial = wl_display_next_serial(d);
        wl_callback_send_done(surf->pending_frame_callback, serial);
        wl_resource_destroy(surf->pending_frame_callback);
        surf->pending_frame_callback = nullptr;

        // Now safe to clear buffer
        surf->buffer_res = nullptr;
    }

    if(compositor != nullptr)
    {

        if (surf->is_xdg_toplevel && !compositor->focused_surface) 
        {
            // sanity checks
            if (!surf->toplevel_res)
            {
                std::cout << "[LumaCompositor] surface_commit: toplevel_res is NULL, skipping initial configure\n";
            } 
            else 
            {
                compositor->focused_surface = surface_res;
            }

            // std::cout << "[Focus] surface_commit compositor->focused_surface = "<<compositor->focused_surface << std::endl;
        }
    }
    else
    {
        std::cout << "[LumaCompositor] ERROR: surface_commit, comp is NULL"<<std::endl;

    }
}

static void surface_destroy(wl_client* /*client*/, wl_resource* resource)
{
    std::cout << "[LumaCompositor] surface_destroy \n";

    wl_resource_destroy(resource);
}

static void surface_damage(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t)
{
    // std::cout << "[LumaCompositor] surface_damage \n";
}

static void surface_frame(wl_client* client, wl_resource* surface_res, uint32_t callback_id)
{
    // std::cout << "[LumaCompositor] surface_frame \n";
    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(surface_res));
    if (!surf) return;

    wl_resource* cb = wl_resource_create(client, &wl_callback_interface,
                                         wl_resource_get_version(surface_res), callback_id);
    if (!cb) return;

    surf->pending_frame_callback = cb;
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

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf) 
    { 
        std::cerr << "[DEBUG] surf NULL in xdg handler\n";
        return; 
    }
    // Prevent double free
    wl_resource_set_user_data(resource, nullptr);

    // Clean up safely
    delete surf;
}


// --------------------------- xdg_surface ---------------------------

// Helper to append a state value into a wl_array
static void push_state(struct wl_array *states, uint32_t value) {
    uint32_t *s = (uint32_t *)wl_array_add(states, sizeof(uint32_t));
    if (s) {
        *s = value;
    }
}

void send_toplevel_configure(struct wl_resource *xdg_toplevel, int w, int h,
                             bool activated, bool resizing, bool maximized, bool fullscreen)
{
    struct wl_array states;
    wl_array_init(&states);

    if (activated)   push_state(&states, XDG_TOPLEVEL_STATE_ACTIVATED);
    if (resizing)    push_state(&states, XDG_TOPLEVEL_STATE_RESIZING);
    if (maximized)   push_state(&states, XDG_TOPLEVEL_STATE_MAXIMIZED);
    if (fullscreen)  push_state(&states, XDG_TOPLEVEL_STATE_FULLSCREEN);

    xdg_toplevel_send_configure(xdg_toplevel, w, h, &states);
    wl_array_release(&states);
}

void compositor_maximize(my_surface *surf, struct wl_display *display,
                         int work_w, int work_h)
{
    send_toplevel_configure(surf->toplevel_res, work_w, work_h,
                            /*activated=*/true, /*resizing=*/false,
                            /*maximized=*/true, /*fullscreen=*/false);

    uint32_t serial = wl_display_next_serial(display);
    xdg_surface_send_configure(surf->xdg_surface_res, serial);
}

void compositor_restore(my_surface *surf, struct wl_display *display,
                        int w, int h)
{
    send_toplevel_configure(surf->toplevel_res, w, h,
                            /*activated=*/true, /*resizing=*/false,
                            /*maximized=*/false, /*fullscreen=*/false);

    uint32_t serial = wl_display_next_serial(display);
    xdg_surface_send_configure(surf->xdg_surface_res, serial);
}

void compositor_fullscreen(my_surface *surf, struct wl_display *display,
                           int mode_w, int mode_h)
{
    send_toplevel_configure(surf->toplevel_res, mode_w, mode_h,
                            /*activated=*/true, /*resizing=*/false,
                            /*maximized=*/false, /*fullscreen=*/true);

    uint32_t serial = wl_display_next_serial(display);
    xdg_surface_send_configure(surf->xdg_surface_res, serial);
}

void compositor_minimize(my_surface *surf, LumaCompositor *comp)
{
    if (comp->focused_surface == surf->resource)
        comp->focused_surface = NULL; // and send wl_keyboard.leave if needed

    surf->visible = false; // your scene-graph flag; skip it in rendering
}

void compositor_close(my_surface *surf)
{
    if (surf->toplevel_res)
        wl_resource_destroy(surf->toplevel_res);
    if (surf->xdg_surface_res)
        wl_resource_destroy(surf->xdg_surface_res);
    // Also remove from scene graph and free your my_surface
}

void compositor_resize_step(my_surface *surf, struct wl_display *display,
                            int new_w, int new_h)
{
    send_toplevel_configure(surf->toplevel_res, new_w, new_h,
                            /*activated=*/true, /*resizing=*/true,
                            /*maximized=*/false, /*fullscreen=*/false);

    uint32_t serial = wl_display_next_serial(display);
    xdg_surface_send_configure(surf->xdg_surface_res, serial);
}




// ------------------ xdg_toplevel ------------------

static void xdg_toplevel_destroy(struct wl_client*, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] xdg_toplevel_destroy\n";
    wl_resource_destroy(resource);
}

static void xdg_toplevel_set_parent(struct wl_client*, struct wl_resource*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_parent\n";
}

static void xdg_toplevel_set_title(struct wl_client*, struct wl_resource*, const char* title)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_title, title = "<<title<<std::endl;
}

static void xdg_toplevel_set_app_id(struct wl_client*, struct wl_resource*, const char*) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_app_id\n";
}

static void xdg_toplevel_show_window_menu(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_resource *seat,
				 uint32_t serial,
				 int32_t x,
				 int32_t y) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_show_window_menu window\n";
}

static void xdg_toplevel_move(struct wl_client* client, struct wl_resource* resource, struct wl_resource* seat, uint32_t serial)
{
    std::cout << "[LumaCompositor] xdg_toplevel_move serial = "<<serial<<std::endl;

    my_surface *surface = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surface)
    {
        std::cout << "[LumaCompositor] xdg_toplevel_move surface is NULL.."<<std::endl;
        return;
    }

    LumaCompositor *comp = surface->compositor;
    if (!comp)
    {
        std::cout << "[LumaCompositor] xdg_toplevel_move Compositor is NULL.."<<std::endl;
        return;
    }

 

    // Store initial positions
    comp->grab_start_x = comp->cursor_x;
    comp->grab_start_y = comp->cursor_y;
    comp->window_start_x = surface->x;
    comp->window_start_y = surface->y;

    // Start "interactive move" mode
    comp->moving_surface = surface;
    comp->move_grab_active = true;
    comp->needs_repaint = true;
    comp->move_grab_serial = serial;
}

static void xdg_toplevel_resize(struct wl_client* client, struct wl_resource* resource, struct wl_resource* seat_res, uint32_t serial, uint32_t edges)
{
    std::cout << "[LumaCompositor] xdg_toplevel_resize\n";


    std::cout << "[LumaCompositor] xdg_toplevel_resize called, edges=" << edges << "\n";

    my_surface *surface = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surface)
    {
        std::cout << "[LumaCompositor] xdg_toplevel_resize surface is NULL.."<<std::endl;
        return;
    }
    LumaCompositor *comp = surface->compositor;
    if (!comp)
    {
        std::cout << "[LumaCompositor] xdg_toplevel_resize Compositor is NULL.."<<std::endl;
        return;
    }
    comp->resizing_surface = surface;
    comp->resize_edges = edges;
    comp->resize_grab_active = true;

    comp->grab_start_x = comp->cursor_x;
    comp->grab_start_y = comp->cursor_y;
    comp->window_start_w = surface->width;
    comp->window_start_h = surface->height;
}

static void xdg_toplevel_set_max_size(struct wl_client*, struct wl_resource*, int32_t, int32_t)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_max_size\n";
}

static void xdg_toplevel_set_min_size(struct wl_client*, struct wl_resource*, int32_t, int32_t) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_min_size\n";
}

static void xdg_toplevel_set_maximized(struct wl_client* client, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_maximized\n";

    // my_surface* surf = (my_surface*)wl_resource_get_user_data(resource);
    // if (!surf) return;
    // LumaCompositor* comp = surf->compositor;
    // if (!comp) return;

    // // idempotent: if already maximized, ignore
    // if (surf->is_maximized)
    // {
    //     std::cout << "[LumaCompositor] xdg_toplevel_set_maximized (already maximized) - ignored\n";
    //     return;
    // }

    // // store current geometry so we can restore on unset
    // surf->restore_x = surf->x;
    // surf->restore_y = surf->y;
    // surf->restore_width = surf->width;
    // surf->restore_height = surf->height;

    // // set maximized geometry
    // surf->is_maximized = true;
    // surf->x = 0;
    // surf->y = 0;
    // surf->width = comp->output_width;
    // surf->height = comp->output_height;

    // std::cout << "[LumaCompositor] xdg_toplevel_set_maximized\n";

    // // Tell the client it is now maximized via configure (include maximized state)
    // send_toplevel_configure(surf->toplevel_res, surf->width, surf->height,
    //                         /*activated=*/true, /*resizing=*/false,
    //                         /*maximized=*/true, /*fullscreen=*/false);

    // // also send xdg_surface.configure so client acks and can resize its buffer
    // uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    // xdg_surface_send_configure(surf->xdg_surface_res, serial);

    // wl_display_flush_clients(comp->display);
}

static void xdg_toplevel_unset_maximized(struct wl_client* client, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] xdg_toplevel_unset_maximized\n";

    my_surface* surf = (my_surface*)wl_resource_get_user_data(resource);
    if (!surf) return;
    LumaCompositor* comp = surf->compositor;
    if (!comp) return;

    if (!surf->is_maximized) {
        std::cout << "[LumaCompositor] xdg_toplevel_unset_maximized (not maximized) - ignored\n";
        return;
    }

    // restore previous geometry
    surf->is_maximized = false;
    surf->x = surf->restore_x;
    surf->y = surf->restore_y;
    surf->width = surf->restore_width;
    surf->height = surf->restore_height;

    std::cout << "[LumaCompositor] xdg_toplevel_unset_maximized\n";

    send_toplevel_configure(surf->toplevel_res, surf->width, surf->height,
                            /*activated=*/true, /*resizing=*/false,
                            /*maximized=*/false, /*fullscreen=*/false);

    uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    xdg_surface_send_configure(surf->xdg_surface_res, serial);

    wl_display_flush_clients(comp->display);
}

static void xdg_toplevel_set_fullscreen(struct wl_client*, struct wl_resource*, struct wl_resource*) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_fullscreen\n";
}

static void xdg_toplevel_unset_fullscreen(struct wl_client*, struct wl_resource*) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_unset_fullscreen\n";
}

static void xdg_toplevel_set_minimized(struct wl_client*, struct wl_resource*) 
{
    std::cout << "[LumaCompositor] xdg_toplevel_set_minimized\n";
}


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

static void destroy_xdg_toplevel_resource(struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] destroy_xdg_toplevel_resource\n";

    my_surface* surf = static_cast<my_surface*>(wl_resource_get_user_data(resource));
    if (!surf) return;
    surf->toplevel_res = nullptr;
    // do not delete surf here — surface_resource_destroy owns lifecycle

    wl_resource_set_user_data(resource, nullptr);
}

// ------------------ xdg_surface ------------------
static void xdg_surface_destroy(struct wl_client*, struct wl_resource* resource)
{
    std::cout << "[LumaCompositor] xdg_surface_destroy...\n";
    wl_resource_destroy(resource);
}



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

    std::cout << "[LumaCompositor] Sending initial configure after get_toplevel\n";
    if (!toplevel)
    {
        std::cerr << "[LumaCompositor] ERROR: toplevel is NULL\n";

        wl_client_post_no_memory(client);
        return;
    }

    // attach the same surface-wrapper (or a new wrapper for toplevel) as user_data
    wl_resource_set_implementation(toplevel, &xdg_toplevel_impl, surf, destroy_xdg_toplevel_resource);


    surf->toplevel_res = toplevel;
    surf->is_xdg_toplevel = true;
    surf->xdg_surface_res = resource; 

    send_toplevel_configure(surf->toplevel_res, 1000, 600, /*activated=*/true,
                        /*resizing=*/false, /*maximized=*/false, /*fullscreen=*/false);

    uint32_t serial = wl_display_next_serial(wl_client_get_display(client));

    xdg_surface_send_configure(resource, serial);
    wl_display_flush_clients(wl_client_get_display(client));
    std::cout << "[LumaCompositor] End xdg_surface_get_toplevel..." << std::endl;
}

static void xdg_surface_get_popup(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] xdg_surface_get_popup...\n";
}

static void xdg_surface_set_window_geometry(struct wl_client*, struct wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    std::cout << "[LumaCompositor] xdg_surface_set_window_geometry...\n";
    my_surface* surface = (my_surface*)wl_resource_get_user_data(resource);

    if(surface != nullptr)
    {
        surface->x = x;
        surface->y = y;
        surface->width = width;
        surface->height = height;
        LumaCompositor* comp = surface->compositor;

        if(comp != nullptr)
        {

            comp->cursor_x = width/2;
            comp->cursor_y = height/2;
            
            comp->focus_x = x;
            comp->focus_y = y;
            comp->focus_width = width;
            comp->focus_height = height;
            std::cout << "[LumaCompositor] xdg_surface_set_window_geometry. x = "<<comp->focus_x<<", y = "<<comp->focus_y<< ", focus_width = "<<comp->focus_width<<", focus_height = "<<comp->focus_height<<std::endl;
        }
    }
}

static void xdg_surface_ack_configure(struct wl_client*, struct wl_resource* resource, uint32_t serial)
{
    std::cout << "[LumaCompositor] xdg_surface_ack_configure serial= "<<serial<<std::endl;


    my_surface* surface = (my_surface*)wl_resource_get_user_data(resource);
    LumaCompositor* comp = surface->compositor;

    surface->configured = true;
    comp->focused_surface = surface->resource;
    std::cout << "[LumaCompositor] xdg_surface_ack_configure comp->focused_surface= "<<comp->focused_surface<<std::endl;

    // Send keyboard enter
    if (comp->keyboard_resource) {
        safe_send_keyboard_enter(comp, comp->focused_surface, comp->display);
    }
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

static void xdg_surface_resource_destroy(struct wl_resource* resource)
{
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

// ------------------ safe wl_region implementation ------------------
// Put this near other interface implementations.

static void region_destroy_impl_cb(struct wl_resource *resource)
{
    // nothing to free right now; libwayland will free resource
    (void)resource;
    std::cout << "[LumaCompositor] region_destroy_impl_cb\n";
}

static void region_destroy_impl(struct wl_client*, struct wl_resource*)
{
    std::cout << "[LumaCompositor] region_destroy_impl...\n";

}


static void region_add_impl(struct wl_client *client, struct wl_resource *resource,
                            int32_t x, int32_t y, int32_t width, int32_t height)
{
    (void)client; (void)resource;
    std::cout << "[LumaCompositor] region_add_impl: " << x << "," << y << " " << width << "x" << height << "\n";
}

static void region_subtract_impl(struct wl_client *client, struct wl_resource *resource,
                                 int32_t x, int32_t y, int32_t width, int32_t height)
{
    (void)client; (void)resource;
    std::cout << "[LumaCompositor] region_subtract_impl\n";
}

static const struct wl_region_interface region_impl = {
    .destroy  = region_destroy_impl,
    .add      = region_add_impl,
    .subtract = region_subtract_impl
};

// --------------------------- wl_compositor ---------------------------

static void compositor_create_surface(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] compositor_create_surface called (compositor_global_res=" << resource << " ) creating surface_res id=" << id << "\n";
    LumaCompositor* comp = (LumaCompositor*)wl_resource_get_user_data(resource);

    // negotiate version: use the version requested by client when created (resource passed is the compositor global resource)
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), wl_surface_interface.version);
    wl_resource* surface_res = wl_resource_create(client, &wl_surface_interface, ver, id);
    if (!surface_res)
    {
        wl_client_post_no_memory(client);
        return;
    }

    my_surface* surf = new my_surface();
    comp->surfaces.push_back(surf);
    surf->resource = surface_res;
    surf->compositor = comp;
    std::cout << "[LumaCompositor] Client created surface comp =" << comp <<std::endl;

    wl_resource_set_implementation(surface_res, &surface_impl, surf, surface_resource_destroy);
    std::cout << "[LumaCompositor] Client created surface (resource=" << surface_res << ")\n";
}

static void compositor_create_region(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    std::cout << "[LumaCompositor] compositor_create_region\n";

    // negotiate version safely
    uint32_t ver = std::min<uint32_t>(wl_resource_get_version(resource), wl_region_interface.version);
    wl_resource *region_res = wl_resource_create(client, &wl_region_interface, ver, id);
    if (!region_res) {
        wl_client_post_no_memory(client);
        return;
    }
    // set implementation (destroy signature must be void (*)(wl_resource*))
    wl_resource_set_implementation(region_res, &region_impl, nullptr, region_destroy_impl_cb);

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

    if (!res) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(res, &compositor_impl, data, nullptr);
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

static void bind_xdg_wm_base(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
    std::cout << "[LumaCompositor] bind_xdg_wm_base...\n";

    wl_resource* res = wl_resource_create(client, &xdg_wm_base_interface, version, id);
    wl_resource_set_implementation(res, &xdg_wm_base_impl, data, nullptr);

    LumaCompositor* compositor = static_cast<LumaCompositor*>(data);
    compositor->wm_base_resource = res;

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

static void sdl_renderer_thread(int win_w, int win_h, LumaCompositor* comp)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return;
    }

    window = SDL_CreateWindow("LumaCompositor (Preview)",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          win_w, win_h,
                                          SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                win_w, win_h);
    if (!texture)
    {
        std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    sdl_thread_running.store(true);

    while (sdl_thread_running.load())
    {
        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                sdl_thread_running.store(false);
            } 
            else if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                // Optionally respond to resize
            }
            else if (ev.type == SDL_MOUSEMOTION)
            {
                int win_x, win_y;
                SDL_GetWindowPosition(window, &win_x, &win_y);   // top-left of window on screen
                int mouse_x_global, mouse_y_global;
                SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);

                // Compute mouse position relative to the SDL window content area
                double local_x = mouse_x_global - win_x;
                double local_y = mouse_y_global - win_y;

                // Optional: scale correction if window != compositor framebuffer
                int win_w, win_h;
                SDL_GetWindowSize(window, &win_w, &win_h);

                double sx = local_x * ((double)comp->output_width  / (double)win_w);
                double sy = local_y * ((double)comp->output_height / (double)win_h);
                comp->cursor_x = sx;
                comp->cursor_y = sy;

                // Move Toplevel
                if (comp->move_grab_active && comp->moving_surface)
                {
                    int dx = comp->cursor_x - comp->grab_start_x;
                    int dy = comp->cursor_y - comp->grab_start_y;

                    comp->moving_surface->x = comp->window_start_x + dx;
                    comp->moving_surface->y = comp->window_start_y + dy;

                    // Repaint with updated position
                    compositor_repaint(comp);
                }
                else if (sx >= 0 && sy >= 0 && sx < comp->output_width && sy < comp->output_height)
                {
                    uint32_t time = SDL_GetTicks();
                    compositorInput.SendMouseMoveEvent(comp, sx, sy, time);
                }
            }
            else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP)
            {
                uint32_t button = 0;
                switch (ev.button.button)
                {
                    case SDL_BUTTON_LEFT: button = BTN_LEFT; break;
                    case SDL_BUTTON_MIDDLE: button = BTN_MIDDLE; break;
                    case SDL_BUTTON_RIGHT: button = BTN_RIGHT; break;
                }

                uint32_t state = (ev.type == SDL_MOUSEBUTTONDOWN)
                                    ? WL_POINTER_BUTTON_STATE_PRESSED
                                    : WL_POINTER_BUTTON_STATE_RELEASED;



                if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_RELEASED)
                {
                    if (comp->move_grab_active && comp->moving_surface)
                    {
                        // finalize move
                        comp->move_grab_active = false;
                        comp->moving_surface = nullptr;

                        // final repaint to ensure the surface appears at the final position
                        std::cout<<"Move Stopped *****************************************"<<std::endl;
                        compositor_repaint(comp);

                        // (optional) send any configure or focus updates as needed
                    }
                }

                
                uint32_t serial = wl_display_next_serial(comp->display);
                compositorInput.SendButtonEvent(comp, serial, SDL_GetTicks(), button, state);
            }
        }

        // Copy compositor framebuffer into texture
        {
            std::lock_guard<std::mutex> lk(comp_fb_mutex);
            // note: comp_framebuffer.size == comp_width*comp_height
            void* pixels = nullptr;
            int pitch = 0;
            if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0)
            {
                // pitch is bytes per row; our comp_width*4 equals expected pitch if sizes match
                uint8_t* dst = (uint8_t*)pixels;
                uint8_t* src = (uint8_t*)comp_framebuffer.data();
                // if comp_width equals texture width and pitch == comp_width*4, we can memcpy whole buffer
                if (pitch == win_w * 4)
                {
                    memcpy(dst, src, win_w * win_h * 4);
                }
                else
                {
                    // copy row by row
                    for (int y = 0; y < win_h; ++y)
                    {
                        memcpy(dst + y * pitch, src + y * win_w * 4, win_h * 4);
                    }
                }
                SDL_UnlockTexture(texture);
            }
        }

        SDL_RenderClear(renderer);
        // Fit texture to window
        SDL_Rect dest;
        int ww, wh;
        SDL_GetWindowSize(window, &ww, &wh);
        dest.x = 0; dest.y = 0; dest.w = ww; dest.h = wh;
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


// ------------------ Init & Run ------------------

void luma_init(LumaCompositor* comp)
{
    std::cout << "[LumaCompositor] init: output_width=" << comp->output_width
          << " output_height=" << comp->output_height << std::endl;

    comp->xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    comp->keymap = xkb_keymap_new_from_names(comp->xkb_ctx, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    comp->xkb_state = xkb_state_new(comp->keymap);

    // comp_framebuffer = (uint32_t*)malloc(comp_width * comp_height * 4);
    // memset(comp_framebuffer, 0x00, comp_width * comp_height * 4);

    comp_framebuffer.assign(comp->output_width * comp->output_height, 0xff000000); // opaque black
    comp->fb_stride = comp->output_width * 4;  // ARGB8888 = 4 bytes/pixel
    // Start SDL preview window at e.g. 1280x720

    std::cout << "[LumaCompositor] framebuffer size: " << comp_framebuffer.size()
          << " (expected " << (comp->output_width * comp->output_height) << ")\n";
    std::cout << "[LumaCompositor] fb_stride = " << comp->fb_stride << std::endl;
    sdl_thread = std::thread(sdl_renderer_thread, comp->output_width, comp->output_height, comp);
    std::cout << "[LumaCompositor] sdl_thread_running = " << sdl_thread_running.load() << std::endl;
    comp->display = wl_display_create();

    compositorInput.Initialize(comp);


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

    comp->loop = wl_display_get_event_loop(comp->display);


    comp->output_global = wl_global_create(comp->display,
                                            &wl_output_interface,
                                            wl_output_interface.version,
                                            comp,
                                            wl_output_handle_bind);

    comp->compositor_global = wl_global_create(comp->display,
                                               &wl_compositor_interface,
                                               4,                           // version from wayland.xml
                                               comp, bind_compositor);
    comp->shm_global = wl_global_create(comp->display,
                                        &wl_shm_interface,
                                        1, comp, bind_shm);
    comp->xdg_wm_base_global = wl_global_create(comp->display,
                                                &xdg_wm_base_interface,
                                                4,                          // version from your xdg-shell.xml
                                                comp, bind_xdg_wm_base);

    // Setup Keyboard and Mouse
    LumaSeat* seat = new LumaSeat();
    comp->seat = seat;
    seat->display = comp->display;
    seat->compositor = comp;
    wl_list_init(&seat->keyboards);
    wl_list_init(&seat->pointers);

    seat->seat_global = wl_global_create(
        comp->display,
        &wl_seat_interface,
        4, // version
        seat,
        seat_bind);

    setup_wayland_display(comp->display);

    std::cout << "[LumaCompositor] Wayland display initialized\n";
}

void luma_run(LumaCompositor* comp)
{
    std::cout << "[LumaCompositor] Running Wayland event loop...\n";
    wl_display_run(comp->display);
}