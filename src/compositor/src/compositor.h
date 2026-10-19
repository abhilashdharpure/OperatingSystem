#pragma once

#include "compositor_input.h"
#include "defines.h"

bool luma_init(LumaCompositor* comp);
void luma_run(LumaCompositor* comp);
void compositor_repaint(LumaCompositor* comp);
void compositor_present(LumaCompositor* comp);

void raise_surface(LumaCompositor*, my_surface*);
void shell_broadcast_all(LumaCompositor*);
void compositor_repaint(LumaCompositor*);

uint32_t get_ticks(void);
void handle_mouse_move(LumaCompositor* comp, double sx, double sy);
CompositorInput* getCompositorInput();