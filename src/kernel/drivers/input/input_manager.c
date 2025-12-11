#include "input_manager.h"
#include "string.h"
#include "debug.h"
#include "arch/i686/events/input_event.h"

static InputManager input_mgr;

void input_manager_init(void)
{
    memset(&input_mgr, 0, sizeof(InputManager));
}

void input_manager_handle_event(int type, int code, int value)
{
    // Type 1 = Keyboard, Type 2 = Mouse motion, Type 4 = Mouse button (depending on your event design)

    switch (type) {
        case EV_KEY: // Keyboard/Mouse button event
            
            if (code < 256)
            {
                input_mgr.keyboard.keys[code] = (value != 0);
            }
            else if (code == BTN_LEFT)
            {
                input_mgr.mouse.left_button   = (value != 0);
            }
            else if (code == BTN_RIGHT)
            {
                 input_mgr.mouse.right_button  = (value != 0);
            }
            else if (code == BTN_MIDDLE)
            {
                input_mgr.mouse.middle_button = (value != 0);
            }
            break;

        case 2: // Relative mouse motion
            if (code == 0) input_mgr.mouse.dx = value; // X delta
            if (code == 1) input_mgr.mouse.dy = value; // Y delta
            input_mgr.mouse.x += input_mgr.mouse.dx;
            input_mgr.mouse.y += input_mgr.mouse.dy;
            if (input_mgr.mouse.x < 0) input_mgr.mouse.x = 0;
            if (input_mgr.mouse.y < 0) input_mgr.mouse.y = 0;

            // if (code == 0) input_mgr.mouse.x += value; // X delta
            // else if (code == 1) input_mgr.mouse.y += value; // Y delta

            // if (input_mgr.mouse.x < 0) input_mgr.mouse.x = 0;
            // if (input_mgr.mouse.y < 0) input_mgr.mouse.y = 0;
            break;

        case 4: // Mouse buttons (depending on your PS/2 event encoding)
            // log_info("Input Manager", "Button preesed from mouse");
            // if (code == 272) input_mgr.mouse.left_button   = (value != 0);
            // if (code == 273) input_mgr.mouse.right_button  = (value != 0);
            // if (code == 274) input_mgr.mouse.middle_button = (value != 0);
            break;

        default:
            break;
    }
}

const MouseState* input_get_mouse(void)
{
    return &input_mgr.mouse;
}

const KeyboardState* input_get_keyboard(void)
{
    return &input_mgr.keyboard;
}