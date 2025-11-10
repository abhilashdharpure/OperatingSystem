#include "compositor_input.h"

#include <thread>
#include <mutex>
#include <xkbcommon/xkbcommon.h>
#include <sys/time.h>
#include "compositor_helper.h"

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
    std::cout << "[Input] Start hit_test_surface, x= "<<x<<", y ="<<y << std::endl;

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

        std::cout << "[Input] Inside for surfaces, x = "<<s->x<<", y = "<<s->y<<", width = "<<s->width<<", height = "<< s->height<< std::endl;

        // test against recorded geometry
        if (x >= s->x && x < s->x + s->width && y >= s->y && y < s->y + s->height)
        {
            std::cout << "[Input] Inside **********" << std::endl;

            return s;
        }
        else
        {
            std::cout << "[Input] Outside " << std::endl;

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
            std::cout << "[Input] inside if (new_surf_res != prev) " << std::endl;

            // tell old focused client it lost keyboard focus
            wl_resource* old_kbd = get_keyboard_for_surface_client(comp, prev);
            if (old_kbd)
            {
                std::cout << "[Input] wl_keyboard_send_leave" << std::endl;

                wl_keyboard_send_leave(old_kbd, wl_display_next_serial(comp->display), prev);
            }
            
            // set new focus and notify
            comp->focused_surface = new_surf_res;
            input->SendKeyboardEnterEvent(comp, comp->focused_surface);

            // wl_resource* new_kbd = get_keyboard_for_surface_client(comp, new_surf_res);
            // if (new_kbd)
            // {
            //     input->SendKeyboardEnterEvent(comp, comp->focused_surface);
            // }
        }
    }
    else
    {
        // // clicked outside any surface -> clear focus
        if (prev) {
        //     wl_resource* old_kbd = get_keyboard_for_surface_client(comp, prev);
        //     if (old_kbd) {
        //         wl_keyboard_send_leave(old_kbd, wl_display_next_serial(comp->display), prev);
        //     }


            comp->focused_surface = nullptr;
            wl_display_flush_clients(comp->display);
        }
    }

    // Also send pointer button events to pointer resources that have entered this client
    // (your existing pointer_send_button loop works if you track pointer enter/leave correctly)
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

                    handle_pointer_button(input, compositor, (int)compositor->cursor_x, (int)compositor->cursor_y, ev.code, ev.value);

                    if (compositor->focused_surface)
                    {
                        uint32_t evdev_code = ev.code;
                        uint32_t xkb_key = evdev_code + 8;
                        uint32_t state = ev.value ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;
                        xkb_state_update_key(compositor->xkb_state, xkb_key, (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? XKB_KEY_DOWN : XKB_KEY_UP);

                        input->SendKeyboardEnterEvent(compositor, compositor->focused_surface);
                    }
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
            else if (ev.type == EV_REL && ev.code == REL_X)
            {
                compositor->cursor_x += ev.value;
                // mouse move X
                static double cursor_x = 0, cursor_y = 0;
                cursor_x += ev.value;



                // Clamp inside screen
                if (compositor->cursor_x < 0) compositor->cursor_x = 0;
                if (compositor->cursor_y < 0) compositor->cursor_y = 0;
                if (compositor->cursor_x >= compositor->output_width) compositor->cursor_x = compositor->output_width - 1;
                if (compositor->cursor_y >= compositor->output_height) compositor->cursor_y = compositor->output_height - 1;


                // std::cout << "[Mouse] compositor->cursor_x= "<<compositor->cursor_x<<", compositor->cursor_y = "<<compositor->cursor_y<<std::endl;

                for (auto* ptr : g_pointers)
                {
                    // std::cout << "[Input] Sending motion " << std::endl;                    

                    wl_pointer_send_motion(ptr,
                        (uint32_t)(ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000),
                        wl_fixed_from_double(cursor_x),
                        wl_fixed_from_double(cursor_y));
                }
            } 
            else if (ev.type == EV_REL && ev.code == REL_Y)
            {
                compositor->cursor_y += ev.value;


                // mouse move Y
                static double cursor_x = 0, cursor_y = 0;
                cursor_y += ev.value;


                                // Clamp inside screen
                if (compositor->cursor_x < 0) compositor->cursor_x = 0;
                if (compositor->cursor_y < 0) compositor->cursor_y = 0;
                if (compositor->cursor_x >= compositor->output_width) compositor->cursor_x = compositor->output_width - 1;
                if (compositor->cursor_y >= compositor->output_height) compositor->cursor_y = compositor->output_height - 1;
                // std::cout << "[Mouse] compositor->cursor_x= "<<compositor->cursor_x<<", compositor->cursor_y = "<<compositor->cursor_y<<std::endl;

                for (auto* ptr : g_pointers)
                {
                    wl_pointer_send_motion(ptr,
                        (uint32_t)(ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000),
                        wl_fixed_from_double(cursor_x),
                        wl_fixed_from_double(cursor_y));
                }
            }
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