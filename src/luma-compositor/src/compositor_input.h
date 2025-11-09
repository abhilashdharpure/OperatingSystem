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
    void SendKeyboardEnterEvent(wl_display* display, wl_resource* resource);

private:
    static void evdev_input_loop(LumaCompositor* compositor);
};