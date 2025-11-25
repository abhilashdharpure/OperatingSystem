#include "compositor_input.h"

#include <thread>
#include <mutex>
#include <xkbcommon/xkbcommon.h>
#include <sys/time.h>
#include "compositor_helper.h"
#include <algorithm>

static constexpr int SCREEN_WIDTH_TEMP = 1920;
static constexpr int SCREEN_HEIGHT_TEMP = 1080;
int count = 1;

static std::atomic<bool> g_running{true};

void CompositorInput::Initialize(LumaCompositor* compositor)
{
    std::cout << "[Input] Start Initialize " << std::endl;
    std::thread(evdev_input_loop, this, compositor).detach();
}

static my_surface* hit_test_surface(LumaCompositor* comp, int32_t x, int32_t y)
{
    // std::cout << "[Input] Start hit_test_surface, x= "<<x<<", y ="<<y << std::endl;

    if (!comp)
    {
        return nullptr;
    }

    // assume surfaces vector is bottom->top; search top to bottom:
    for (auto it = comp->surfaces.rbegin(); it != comp->surfaces.rend(); ++it)
    {
        my_surface* s = *it;
        if (!s || !s->resource)
        {
            continue;
        }

        // std::cout << "[Input] Inside for surfaces, x = "<<s->x<<", y = "<<s->y<<", width = "<<s->width<<", height = "<< s->height<< std::endl;

        // test against recorded geometry
        if (x >= s->x && x < s->x + s->width && y >= s->y && y < s->y + s->height)
        {
            // std::cout << "[Input] Inside **********, X ="<<s->x << std::endl;
            return s;
        }
        else
        {
            // std::cout << "[Input] Outside X = "<<s->x << std::endl;
        }
    }
    return nullptr;
}

// returns keyboard wl_resource* for the client that owns `surface_res` (or nullptr)
static wl_resource* get_keyboard_for_surface_client(LumaCompositor* comp, wl_resource* surface_res)
{
    if (!surface_res)
    {
        return nullptr;
    }
    std::scoped_lock lk(comp->kbd_mutex); 
    wl_client* surface_client = wl_resource_get_client(surface_res);

    struct wl_resource *kbd_res;
    wl_list_for_each(kbd_res, &comp->seat->keyboards, link)
    {
        if (wl_resource_get_client(kbd_res) == surface_client)
        {
            return kbd_res;
        }
    }
    return nullptr;
}

bool isClickOnTopBar(int x, int y, int surface_x, int surface_y, int surface_width, int surface_height)
{
    int topbar_height = 30;
    return ( (x > surface_x) && (x < (surface_x + surface_width)) && (y > surface_y) && ((y< surface_y +  topbar_height)));
}


// called from your evdev input loop on BTN_LEFT press
void handle_pointer_button(CompositorInput* input, LumaCompositor* comp, int x, int y, uint32_t button, uint32_t state)
{
    my_surface* under = hit_test_surface(comp, x, y);
    wl_resource* prev = comp->focused_surface;

    if (under)
    {
        wl_resource* new_surf_res = under->resource;
        if (new_surf_res != prev)
        {
            if( state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                std::cout<<"handle_pointer_button x = "<<x<<", y = "<<y << ", under->x = "<<under->x << ", under->y = "<<under->y<<std::endl;
                // if(!isClickOnTopBar(x, y, under->x , under->y, under->width, under->height))
                // {
                    // set new focus and notify
                    comp->focused_surface = new_surf_res;
                    input->SendKeyboardEnterEvent(comp, comp->focused_surface);
                // }
                // else
                // {
                //     std::cout<<"Clicked on Topbar !!!!!!!!"<<std::endl;
                // }
            }
        }
    }
    else
    {
        // // clicked outside any surface -> clear focus
        if (prev)
        {
            input->SendKeyboardLeaveEvent(comp, prev);

            // wl_resource* old_kbd = get_keyboard_for_surface_client(comp, prev);
            // if (old_kbd)
            // {
            //     input->SendKeyboardLeaveEvent(comp, old_kbd, prev);
            //     // // std::cout << "[Input] Keyboard Leave" << std::endl;
            //     // wl_keyboard_send_leave(old_kbd, wl_display_next_serial(comp->display), prev);
            // }

            comp->focused_surface = nullptr;
            // wl_display_flush_clients(comp->display);
        }
    }
}

void handle_pointer_motion(LumaCompositor* comp, int x, int y)
{
    wl_resource *prev_surface = comp->pointer_focused_surface;
    my_surface* surface = hit_test_surface(comp, x, y);

    if(surface != nullptr)
    {
        wl_resource *new_surface = surface->resource;

        if (new_surface != prev_surface)
        {
            // Enter new surface
            if (new_surface)
            {
                if(!comp->mouse_pressed)
                {
                    uint32_t serial = wl_display_next_serial(comp->display);

                    double sx = x - surface->x;
                    double sy = y - surface->y;

                    wl_resource* focused_pointer = get_focused_pointer(comp);
                    if(focused_pointer)
                    {
                        std::cout << "[Input] handle_pointer_motion Entering new surface, x = "<<x<<", y ="<<y <<", sx = "<< sx<<", sy = "<<sy<< std::endl;
                        wl_pointer_send_enter(focused_pointer, serial, new_surface,
                                            wl_fixed_from_double(sx),
                                            wl_fixed_from_double(sy));
                        wl_pointer_send_frame(focused_pointer);
                    }
                }

                // Update pointer focus
                comp->pointer_focused_surface = new_surface;
            }

        }
    }
    else
    {
        if(!comp->mouse_pressed)
        {
            uint32_t serial = wl_display_next_serial(comp->display);

            wl_resource* focused_pointer = get_focused_pointer(comp);
            if(focused_pointer)
            {
                // Leave previous surface
                if (prev_surface)
                {
                    std::cout << "[Input] handle_pointer_motion Leaving new surface" << std::endl;
                    wl_pointer_send_leave(focused_pointer, serial, prev_surface);
                    wl_pointer_send_frame(focused_pointer);
                }
            }
        }

        comp->pointer_focused_surface = nullptr;
    }
    wl_display_flush_clients(comp->display);

}

void CompositorInput::evdev_input_loop(CompositorInput* input, LumaCompositor* compositor)
{
    std::cout << "[Input] Start evdev_input_loop " << std::endl;

    std::vector<int> fds;

    // Open all /dev/input/event* files (simple version)
    for (int i = 0; i < 32; ++i) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        // std::cout << "[Input] Start evdev_input_loop, path = "<<path<<" fd = "<<fd << std::endl;

        if (fd >= 0) {
            std::cout << "[Input] Opened " << path << std::endl;
            fds.push_back(fd);
        }
    }

    // bool isTerminalClicked = false;

    struct pollfd pfds[32];
    while (g_running)
    {
        int nfds = 0;
        for (int fd : fds) {
            pfds[nfds].fd = fd;
            pfds[nfds].events = POLLIN;
            nfds++;
        }

        int ret = poll(pfds, nfds, 10);
        if (ret <= 0)
        {
            continue;
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (!(pfds[i].revents & POLLIN)) 
            {
                continue;
            }

            struct input_event ev;
            ssize_t n = read(pfds[i].fd, &ev, sizeof(ev));
            if (n != sizeof(ev))
            {
                continue;
            }

            if (ev.type == EV_KEY)
            {
                if ((ev.code == BTN_LEFT || ev.code == BTN_RIGHT))
                {
                    // std::cout << "[Mouse] Mouse Clicked"<<std::endl;

                    // handle_pointer_button(input, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, ev.code, ev.value);

                    // if (compositor->focused_surface)
                    // {
                    //     uint32_t evdev_code = ev.code;
                    //     uint32_t xkb_key = evdev_code + 8;
                    //     uint32_t state = ev.value ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;
                    //     xkb_state_update_key(compositor->xkb_state, xkb_key, (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP);

                    //     input->SendKeyboardEnterEvent(compositor, compositor->focused_surface);

                    //     uint32_t serial = wl_display_next_serial(compositor->display);
                    //     uint32_t time_ms = ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;

                    //     wl_resource* focused_pointer = get_focused_pointer(compositor);
                    //     if(focused_pointer)
                    //     {

                    //         wl_pointer_send_button(focused_pointer, time_ms, serial, ev.code, state);
                    //         wl_pointer_send_frame(focused_pointer);
                    //     }
                    // }
                }
                else
                {
                    // std::cout << "[Keuboard] Key Clicked"<<std::endl;
                    std::scoped_lock lk(compositor->kbd_mutex); 

                    // who is focused?
                    if (!compositor->focused_surface)
                    {
                        // no focused client -> ignore keys for now
                        continue;
                    }

                    // wl_resource* focused_kbd = get_focused_keyboard_for_compositor(compositor);
                    wl_resource* focused_kbd = get_focused_keyboard(compositor);

                    // if (focused_kbd && isTerminalClicked)
                    if (focused_kbd)
                    {
                        uint32_t xkb_key = ev.code ;
                        uint32_t state = ev.value ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;

                        xkb_state_update_key(compositor->xkb_state, xkb_key,
                            state == WL_KEYBOARD_KEY_STATE_PRESSED ? XKB_KEY_DOWN : XKB_KEY_UP);

                        xkb_mod_mask_t depressed = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_DEPRESSED);
                        xkb_mod_mask_t latched   = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_LATCHED);
                        xkb_mod_mask_t locked    = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_LOCKED);
                        xkb_layout_index_t group = xkb_state_serialize_layout(compositor->xkb_state, (xkb_state_component)XKB_STATE_EFFECTIVE);

                        uint32_t serial = wl_display_next_serial(compositor->display);
                        wl_resource* focused_kbd = get_focused_keyboard(compositor);
                        std::cout<<"focused_kbd = "<<focused_kbd<<std::endl;
                        if (focused_kbd)
                        {
                            wl_keyboard_send_modifiers(focused_kbd, serial, depressed, latched, locked, group);

                            uint32_t time_ms = ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
                            wl_keyboard_send_key(focused_kbd, serial, time_ms, xkb_key, state);

                            wl_resource* focused_pointer = get_focused_pointer(compositor);
                            if(focused_pointer)
                            {
                                wl_pointer_send_button(focused_pointer, time_ms, serial, ev.code, state);
                            }
                        }

                        // std::cout << "[Input] Sent key " << xkb_key << " state " << state << " to focused client\n";
                    } 
                    else 
                    {
                        // std::cout << "[Input] Ignored key, no focused surface\n";
                    }

                    wl_display_flush_clients(compositor->display);
                }
            }
        }
    }

    for (int fd : fds)
    {
        close(fd);
    }
}

void CompositorInput::SendKeyboardEnterEvent(LumaCompositor* compositor, wl_resource* focused_surface)
{   

    if((compositor == nullptr) || (focused_surface == nullptr))
    {
        std::cout << "[Input] ERROR: SendKeyboardEnterEvent compositor or focused_surface is NULL" << std::endl;
        return;
    }

    uint32_t serial = wl_display_next_serial(compositor->display);
    struct wl_array empty_keys;
    wl_array_init(&empty_keys);

    wl_resource* focused_kbd = get_focused_keyboard(compositor);

    if(focused_kbd)
    {
        std::cout << "[Input] SendKeyboardEnterEvent" << std::endl;

        wl_keyboard_send_enter(focused_kbd, serial, focused_surface, &empty_keys);

        xkb_mod_mask_t depressed = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_DEPRESSED);
        xkb_mod_mask_t latched   = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_LATCHED);
        xkb_mod_mask_t locked    = xkb_state_serialize_mods(compositor->xkb_state, (xkb_state_component)XKB_STATE_LOCKED);
        xkb_layout_index_t group = xkb_state_serialize_layout(compositor->xkb_state, (xkb_state_component)XKB_STATE_EFFECTIVE);

        wl_keyboard_send_modifiers(focused_kbd, serial, depressed, latched, locked, group);
    }

    wl_array_release(&empty_keys);

    wl_display_flush_clients(compositor->display);

}

void CompositorInput::SendKeyboardLeaveEvent(LumaCompositor* compositor,  wl_resource* prev)
{   
    // std::cout << "[Input] SendKeyboardLeaveEvent" << std::endl;

    if((compositor == nullptr) || (prev == nullptr))
    {
        std::cout << "[Input] ERROR: SendKeyboardLeaveEvent compositor or prev surface is NULL" << std::endl;
        return;
    }


    wl_resource* old_kbd = get_keyboard_for_surface_client(compositor, prev);
    if (old_kbd)
    {

        wl_resource* focused_pointer = get_focused_pointer(compositor);

        if(focused_pointer)
        {
            wl_pointer_send_frame(focused_pointer);
        }

        uint32_t serial = wl_display_next_serial(compositor->display);
        std::cout << "[Input] Keyboard Leave" << std::endl;
        wl_keyboard_send_leave(old_kbd, serial, prev);
        wl_display_flush_clients(compositor->display);
    }
}


void CompositorInput::SendMouseMoveEvent(LumaCompositor* compositor, double gx, double gy, uint32_t time_ms)
{
    // std::cout << "[Mouse] SendMouseMoveEvent cursor_x= "<<gx<<", cursor_y = "<<gy<<std::endl;

    handle_pointer_motion(compositor, gx, gy);

    // When mouse curcer inside client then only send motion
    if(compositor->pointer_focused_surface != nullptr)
    {
        my_surface *surface = static_cast<my_surface*>(wl_resource_get_user_data(compositor->pointer_focused_surface));

        if(surface != nullptr)
        {
            double sx = gx - surface->x;
            double sy = gy - surface->y;

            wl_resource* focused_pointer = get_focused_pointer(compositor);
            if(focused_pointer)
            {
                // std::cout << "[Mouse] wl_pointer_send_motion cursor_x= "<<sx<<", cursor_y = "<<sy<<std::endl;

                wl_pointer_send_motion(focused_pointer, time_ms, wl_fixed_from_double(sx), wl_fixed_from_double(sy));
                wl_pointer_send_frame(focused_pointer);
            }
        }
    }
    wl_display_flush_clients(compositor->display);

}

void CompositorInput::SendButtonEvent(LumaCompositor* compositor, uint32_t time_ms, uint32_t button, uint32_t state)
{
    if(button == BTN_LEFT)
    {
        // handle_pointer_button(this, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, button, state);

        uint32_t serial = wl_display_next_serial(compositor->display);

        wl_resource* focused_pointer = get_focused_pointer(compositor);
        if(focused_pointer)
        {
            std::cout << "[Input] wl_pointer_send_button serial = "<<serial <<", button = "<<button<<", state = "<<state<<std::endl;
            //if (focused_pointer != compositor->pointer_focused_surface) continue;
            wl_pointer_send_button(focused_pointer, serial, time_ms, button, state);
            wl_pointer_send_frame(focused_pointer);
        }
        wl_display_flush_clients(compositor->display);

        compositor->last_press_serial = serial;
        handle_pointer_button(this, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, button, state);
    }
}
