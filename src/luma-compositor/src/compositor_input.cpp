#include "compositor_input.h"

#include <thread>
#include <mutex>
#include <xkbcommon/xkbcommon.h>
#include <sys/time.h>
#include "compositor_helper.h"

static constexpr int SCREEN_WIDTH_TEMP = 1920;
static constexpr int SCREEN_HEIGHT_TEMP = 1080;
int count = 1;

// We'll keep references to active keyboard and pointer resources
static std::vector<wl_resource*> g_keyboards;
static std::vector<wl_resource*> g_pointers;
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
            // std::cout << "[Input] Inside **********" << std::endl;
            return s;
        }
        else
        {
            // std::cout << "[Input] Outside " << std::endl;
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

// Example: send enter + button + frame when clicking a surface
void send_pointer_click_to_surface(LumaCompositor* comp, wl_resource* pointer_res, wl_resource* surface_res, int32_t global_x, int32_t global_y, uint32_t button_code, uint32_t value)
{
    // compute surface-local coords
    uint32_t serial = 0;
    
    if(comp->pointer_enter_serial != 0)
    {
        // For Move toplevel
        serial = comp->pointer_enter_serial;
        comp->pointer_enter_serial = 0;
    }
    else
    {
        serial = wl_display_next_serial(comp->display);
    }
    
    if(value == 1 )
    {
        uint32_t time_ms = (uint32_t)((uint64_t)time(nullptr) * 1000);
        wl_pointer_send_button(pointer_res, time_ms, serial, button_code, value);

        // frame to finish the input event
        wl_pointer_send_frame(pointer_res);

        // std::cout << "[Input] Mouse Clicked, serial = "<<serial<< std::endl;
    }
    else
    {
        // std::cout << "[Input] Mouse Released"<< std::endl;
    }
    


    // flush
    wl_display_flush_clients(comp->display);
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
                // set new focus and notify
                comp->focused_surface = new_surf_res;
                input->SendKeyboardEnterEvent(comp, comp->focused_surface);
            }
        }
    }
    else
    {
        // // clicked outside any surface -> clear focus
        if (prev)
        {
            wl_resource* old_kbd = get_keyboard_for_surface_client(comp, prev);
            if (old_kbd)
            {
                // std::cout << "[Input] Keyboard Leave" << std::endl;
                wl_keyboard_send_leave(old_kbd, wl_display_next_serial(comp->display), prev);
            }

            comp->focused_surface = nullptr;
            wl_display_flush_clients(comp->display);
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
            comp->pointer_enter_serial = wl_display_next_serial(comp->display);

            for (auto* pointer : g_pointers)
            {
                // Enter new surface
                if (new_surface)
                {
                    std::cout << "[Input] handle_pointer_motion Entering new surface, serial = "<<comp->pointer_enter_serial << std::endl;
                    wl_pointer_send_enter(pointer, comp->pointer_enter_serial, new_surface,
                                        wl_fixed_from_double(x),
                                        wl_fixed_from_double(y));
                    wl_pointer_send_frame(pointer);
                }
            }

            // Update pointer focus
            comp->pointer_focused_surface = new_surface;
        }
    }
    else
    {
        uint32_t serial = wl_display_next_serial(comp->display);
        comp->pointer_enter_serial = 0;
        for (auto* pointer : g_pointers)
        {
            // Leave previous surface
            if (prev_surface)
            {
                std::cout << "[Input] handle_pointer_motion Leaving new surface" << std::endl;
                wl_pointer_send_leave(pointer, serial, prev_surface);
                wl_pointer_send_frame(pointer);
            }
        }

        comp->pointer_focused_surface = nullptr;
    }
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
                //     // std::cout << "[Mouse] Mouse Clicked"<<std::endl;

                //     handle_pointer_button(input, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, ev.code, ev.value);

                //     if (compositor->focused_surface)
                //     {
                //         uint32_t evdev_code = ev.code;
                //         uint32_t xkb_key = evdev_code + 8;
                //         uint32_t state = ev.value ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;
                //         xkb_state_update_key(compositor->xkb_state, xkb_key, (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP);

                //         input->SendKeyboardEnterEvent(compositor, compositor->focused_surface);

                //         uint32_t serial = wl_display_next_serial(compositor->display);
                //         uint32_t time_ms = ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
                //         for (auto* ptr : g_pointers)
                //         {

                //             wl_pointer_send_button(ptr, time_ms, serial, ev.code, state);
                //             wl_pointer_send_frame(ptr);
                //         }
                //     }
                }
                else
                {
                    // std::cout << "[Keuboard] Key Clicked"<<std::endl;
 
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

                        if (focused_kbd)
                        {
                            wl_keyboard_send_modifiers(focused_kbd, serial, depressed, latched, locked, group);

                            uint32_t time_ms = ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
                            wl_keyboard_send_key(focused_kbd, serial, time_ms, xkb_key, state);
                            for (auto* ptr : g_pointers)
                            {
                                wl_pointer_send_button(ptr, time_ms, serial, ev.code, state);
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
            // else if (ev.type == EV_REL && ev.code == REL_X)
            // {
            //     compositor->cursor_x += ev.value;
            //     // mouse move X

            //     // Clamp inside screen
            //     if (compositor->cursor_x < 0) compositor->cursor_x = 0;
            //     if (compositor->cursor_y < 0) compositor->cursor_y = 0;
            //     if (compositor->cursor_x >= compositor->output_width) compositor->cursor_x = compositor->output_width - 1;
            //     if (compositor->cursor_y >= compositor->output_height) compositor->cursor_y = compositor->output_height - 1;
            //     // if (compositor->cursor_x >= SCREEN_WIDTH_TEMP) compositor->cursor_x = SCREEN_WIDTH_TEMP - 1;
            //     // if (compositor->cursor_y >= SCREEN_HEIGHT_TEMP) compositor->cursor_y = SCREEN_HEIGHT_TEMP - 1;

            //     // As we have SDL window and ubuntu application icons on left side
            //     // double cursor_x = (compositor->cursor_x - 80) < 0 ? 0: compositor->cursor_x; 
            //     // double cursor_y = (compositor->cursor_y - 80) < 0 ? 0: compositor->cursor_y;
            //     double cursor_x = compositor->cursor_x; 
            //     double cursor_y = compositor->cursor_y; 
            //     // std::cout << "[Mouse] compositor->cursor_x= "<<cursor_x<<", compositor->cursor_y = "<<cursor_y<<std::endl;
                
            //     for (auto* ptr : g_pointers)
            //     {
            //         // std::cout << "[Input] Sending motion " << std::endl;                    

            //         wl_pointer_send_motion(ptr,
            //             (uint32_t)(ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000),
            //             wl_fixed_from_double(cursor_x),
            //             wl_fixed_from_double(cursor_y));
            //         wl_pointer_send_frame(ptr);
            //     }
            // } 
            // else if (ev.type == EV_REL && ev.code == REL_Y)
            // {
            //     // mouse move Y
            //     compositor->cursor_y += ev.value;

            //     // Clamp inside screen
            //     if (compositor->cursor_x < 0) compositor->cursor_x = 0;
            //     if (compositor->cursor_y < 0) compositor->cursor_y = 0;
            //     if (compositor->cursor_x >= compositor->output_width) compositor->cursor_x = compositor->output_width - 1;
            //     if (compositor->cursor_y >= compositor->output_height) compositor->cursor_y = compositor->output_height - 1;
            //     // if (compositor->cursor_x >= SCREEN_WIDTH_TEMP) compositor->cursor_x = SCREEN_WIDTH_TEMP - 1;
            //     // if (compositor->cursor_y >= SCREEN_HEIGHT_TEMP) compositor->cursor_y = SCREEN_HEIGHT_TEMP - 1;

            //     // As we have SDL window and ubuntu application icons on left side
            //     //double cursor_x = (compositor->cursor_x - 80) < 0 ? 0: compositor->cursor_x; 
            //     //double cursor_y = (compositor->cursor_y - 80) < 0 ? 0: compositor->cursor_y; 
            //     double cursor_x = compositor->cursor_x; 
            //     double cursor_y = compositor->cursor_y; 
            //     // std::cout << "[Mouse] compositor->cursor_x= "<<cursor_x<<", compositor->cursor_y = "<<cursor_y<<std::endl;

            //     for (auto* ptr : g_pointers)
            //     {
            //         wl_pointer_send_motion(ptr,
            //             (uint32_t)(ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000),
            //             wl_fixed_from_double(cursor_x),
            //             wl_fixed_from_double(cursor_y));
            //         wl_pointer_send_frame(ptr);
            //     }
            // }
        }
    }

    for (int fd : fds)
    {
        close(fd);
    }
}

void CompositorInput::AddKeyboardEvent(wl_resource* keyboard_res)
{
    std::cout << "[Input] AddKeyboardEvent " << std::endl;                    
    g_keyboards.push_back(keyboard_res);

}

void CompositorInput::AddKeyMouseEvent(wl_resource* mouse_res)
{
    std::cout << "[Input] AddKeyMouseEvent " << std::endl;                    

    g_pointers.push_back(mouse_res);
}

void CompositorInput::SendKeyboardEnterEvent(LumaCompositor* compositor, wl_resource* focused_surface)
{   
    std::cout << "[Input] SendKeyboardEnterEvent" << std::endl;

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

void CompositorInput::SendMouseMoveEvent(LumaCompositor* compositor, double gx, double gy, uint32_t time_ms)
{
    // std::cout << "[Mouse] SendMouseMoveEvent cursor_x= "<<gx<<", cursor_y = "<<gy<<std::endl;
    handle_pointer_motion(compositor, gx, gy);

    // When mouse curcer inside client then only send motion
    if(compositor->pointer_focused_surface != nullptr)
    {
        for (auto* ptr : g_pointers)
        {
            if (ptr)
            {
                wl_pointer_send_motion(ptr, time_ms, wl_fixed_from_double(gx), wl_fixed_from_double(gy));
                wl_pointer_send_frame(ptr);
            }
        }
    }
}

void CompositorInput::SendButtonEvent(LumaCompositor* compositor, uint32_t passed_serial, uint32_t time_ms, uint32_t button, uint32_t state)
{
    if(button == BTN_LEFT)
    {
        // determine the serial to use:
        uint32_t serial = 0;

        // prefer the current pointer-enter serial if pointer is in a surface
        if (compositor->pointer_focused_surface && compositor->pointer_enter_serial != 0)
        {
            serial = compositor->pointer_enter_serial;
        }
        else if (passed_serial != 0)
        {
            serial = passed_serial;
        }
        else
        {
            serial = wl_display_next_serial(compositor->display);
        }



        handle_pointer_button(this, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, button, state);

        // if (compositor->focused_surface)
        // {
        //     xkb_state_update_key(compositor->xkb_state, button, (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP);

        //     SendKeyboardEnterEvent(compositor, compositor->focused_surface);
        // }


        // for (auto* ptr : g_pointers)
        // {
        //     send_pointer_click_to_surface(comp, ptr, under->resource, x, y, button, state);
        // }

        for (auto* ptr : g_pointers)
        {
            if (ptr)
            {
                wl_pointer_send_button(ptr, serial, time_ms, button, state);
                wl_pointer_send_frame(ptr);
            }
        }
            
    }
}