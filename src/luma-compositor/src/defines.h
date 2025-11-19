#pragma once

#include <wayland-server.h>
#include <list>
#include <vector>

// Forward declarations
struct my_output;
struct LumaSeat;
struct my_surface;
struct shm_buffer;


// enum class toplevel_edges
// {
//     TOP = 1,
//     BOTTOM = 2,
//     LEFT = 4,
//     RIGHT = 8,
//     TOP_LEFT = 5,
//     TOP_RIGHT = 9,
//     BOTTOM_RIGHT = 10,
//     BOTTOM_LEFT = 6
// }


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
    wl_resource* wm_base_resource= nullptr;
    wl_resource* cursor_pending_buffer = nullptr;


    LumaSeat* seat;
    
    std::list<my_output*> client_outputs;

    double cursor_x = 0.0;
    double cursor_y = 0.0;
    double cursor_w = 0.0;
    double cursor_h = 0.0;
    bool mouse_pressed = false;
    int output_width =  1200; //1920;
    int output_height = 800; //1080;

    int32_t focus_x = 0;
    int32_t focus_y = 0;
    int32_t focus_width = 0;
    int32_t focus_height = 0;

    uint32_t last_press_serial = 0;
    bool needs_repaint = false;
    bool is_pong_received = true;

    // For Toplevel Move
    my_surface* moving_surface = nullptr;
    my_surface* new_client_surface = nullptr;
    bool move_grab_active = false;
    double grab_start_x = 0.0;
    double grab_start_y = 0.0;
    double window_start_x = 0.0;
    double window_start_y = 0.0;
    uint32_t move_grab_serial = 0;

    // For Toplevel Resize
    my_surface* resizing_surface = nullptr;
    bool resize_grab_active = false;
    double window_start_w = 0.0;
    double window_start_h = 0.0;
    uint32_t resize_edges = 0;

    // Cursor surca
    my_surface* cursor_surface = nullptr;
    bool is_cursor_surface = false;
    double cursor_hot_x = 0.0;
    double cursor_hot_y = 0.0;

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
    wl_resource* pending_frame_callback = nullptr;

    shm_buffer* committed_buffer = nullptr; // store latest committed buffer

    // geometry we will use for hit-testing and configure
    int32_t x = 0, y = 0;   // top-left position on the compositor output
    int32_t width = 0, height = 0; // size the compositor gave via xdg_toplevel_send_configure
    bool configured = false;     
    bool is_xdg_toplevel = false;             // true when xdg_toplevel created for this surface
    bool visible = false;
    bool is_maximized = false;

    int window_geom_x, window_geom_y; // offset inside buffer
    int window_geom_w, window_geom_h;

    // Restore for Maximize -> Minimize
    int32_t restore_x = 0;
    int32_t restore_y = 0;
    int32_t restore_width = 0;
    int32_t restore_height = 0;

    bool mapped = false;
};

struct my_output {
    wl_resource* resource = nullptr;
    LumaCompositor* state = nullptr;

    int width = 1200; //1920;
    int height = 800; //1080;
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
    int width = 0;
    int height = 0;
    int stride = 0;
    uint32_t format = 0;

    my_surface* owner_surface = nullptr;
};

struct LumaSeat {
    wl_global* seat_global;
    wl_list keyboards; // linked list of wl_resources
    wl_list pointers;
    wl_display* display;

    wl_resource *pointer_res; 
    wl_resource *pointer_focus_surface; // wl_surface the pointer is over
    uint32_t last_pointer_serial;
    bool grab_active;
    int grab_offset_x, grab_offset_y;

    LumaCompositor* compositor;
};

struct keymap_fd_holder {
    int fd;
    struct wl_listener listener;    
};
