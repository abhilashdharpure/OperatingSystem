#pragma once

#include <fcntl.h>
#include <input_events.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <thread>
#include <iostream>
#include <atomic>
#include <wayland-server.h>
#include "wayland-protocol.h"
#include "defines.h"


class CompositorInput
{
public:

    void Initialize(LumaCompositor* compositor);
    void SendKeyboardEnterEvent(LumaCompositor* compositor, wl_resource* focused_surface);
    void SendKeyboardLeaveEvent(LumaCompositor* compositor, wl_resource* prev);
    void SendMouseMoveEvent(LumaCompositor* compositor, double gx, double gy, uint32_t time_ms);
    // void SendButtonEvent(LumaCompositor* compositor, uint32_t time_ms, uint32_t button, uint32_t state);

private:
    static void evdev_input_loop(CompositorInput* input, LumaCompositor* compositor);
//     static void compositor_keyboard_event(LumaCompositor *comp, InputEvent *ev);
//     static void compositor_mouse_event(LumaCompositor *comp, InputEvent *ev);

// static int keyboard_fd_handler(
//     int fd,
//     uint32_t mask,
//     void *data);

// static int mouse_fd_handler(
//     int fd,
//     uint32_t mask,
//     void *data);
};