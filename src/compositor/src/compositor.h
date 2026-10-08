#pragma once

#include "compositor_input.h"
#include "defines.h"

bool luma_init(LumaCompositor* comp);
void luma_run(LumaCompositor* comp);


uint32_t get_ticks(void);
void handle_mouse_move(LumaCompositor* comp, double sx, double sy);
CompositorInput* getCompositorInput();