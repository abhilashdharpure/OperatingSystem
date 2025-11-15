#pragma once

#include <fcntl.h>
#include <linux/input.h>
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

    void AddKeyboardEvent(wl_resource* keyboard_res);
    void AddKeyMouseEvent(wl_resource* mouse_res);
    void SendKeyboardEnterEvent(LumaCompositor* compositor, wl_resource* focused_surface);
    void SendMouseMoveEvent(LumaCompositor* compositor, double gx, double gy, uint32_t time_ms);
    void SendButtonEvent(LumaCompositor* compositor, uint32_t serial, uint32_t time_ms, uint32_t button, uint32_t state);

private:
    static void evdev_input_loop(CompositorInput* input, LumaCompositor* compositor);
};