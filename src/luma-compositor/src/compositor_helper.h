#pragma once

#include <wayland-server.h>
#include "wayland-protocol.h"
#include "defines.h"

wl_resource* get_focused_keyboard(LumaCompositor* compositor);