#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    bool left_button;
    bool right_button;
    bool middle_button;
} MouseState;

typedef struct {
    bool keys[256];   // true if pressed
} KeyboardState;

typedef struct {
    MouseState mouse;
    KeyboardState keyboard;
} InputManager;

void input_manager_init(void);
void input_manager_handle_event(int type, int code, int value);
const MouseState* input_get_mouse(void);
const KeyboardState* input_get_keyboard(void);