#include "compositor/compositor.h"
#include "drivers/fb/fb.h"
#include "drivers/input/input_manager.h"
#include <util/string.h> 
#include "debug.h"

#define MAX_SURFACES 16

// ----------------------------------------------------
// Global state
// ----------------------------------------------------
static Surface surfaces[MAX_SURFACES];
static int surface_count = 0;

static int mouse_x = 100;
static int mouse_y = 100;
static const int mouse_size = 10; // small blue cursor square

// ----------------------------------------------------
// Initialize compositor
// ----------------------------------------------------
void compositor_init(void)
{
    log_info("COMPOSITOR", "Initializing...");

    // Clear framebuffer to black
    fb_clear(COMPOSITOR_BG_COLOR);

    // Reset surface list
    surface_count = 0;
    memset(surfaces, 0, sizeof(surfaces));

    // Create a test window (green square)
    compositor_create_surface(200, 150, 100, 100, RGB(0, 255, 0));

    log_info("COMPOSITOR", "Initialized framebuffer %dx%d", fb_dev.width, fb_dev.height);
}

// ----------------------------------------------------
// Create a surface (simple colored rectangle)
// ----------------------------------------------------
Surface* compositor_create_surface(int x, int y, int w, int h, uint32_t color)
{
    if (surface_count >= MAX_SURFACES)
        return NULL;

    Surface *s = &surfaces[surface_count++];
    s->x = x;
    s->y = y;
    s->width = w;
    s->height = h;
    s->color = color;
    s->visible = true;

    return s;
}

// ----------------------------------------------------
// Redraw all visible surfaces + cursor
// ----------------------------------------------------
void compositor_redraw(void)
{
    fb_clear(COMPOSITOR_BG_COLOR);

    // Draw all visible surfaces
    for (int i = 0; i < surface_count; i++)
    {
        if (surfaces[i].visible)
            fb_draw_rect(surfaces[i].x, surfaces[i].y,
                         surfaces[i].width, surfaces[i].height,
                         surfaces[i].color);
    }

    // Draw mouse cursor (blue block)
    fb_draw_rect(mouse_x, mouse_y, mouse_size, mouse_size, RGB(0, 0, 255));
}

// ----------------------------------------------------
// Handle input (keyboard + mouse)
// ----------------------------------------------------
void compositor_handle_input(InputEvent *ev)
{
    if (ev->type == 2) // relative mouse movement
    { 
        if (ev->code == 0) mouse_x += ev->value; // X move
        if (ev->code == 1) mouse_y += ev->value; // Y move
    }

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > fb_dev.width - mouse_size) mouse_x = fb_dev.width - mouse_size;
    if (mouse_y > fb_dev.height - mouse_size) mouse_y = fb_dev.height - mouse_size;


    // InputEvent ev;
    // while (input_pop_event(&keyboard_dev, &ev)) // for keyboard
    //     input_manager_handle_event(ev.type, ev.code, ev.value);

    // while (input_pop_event(&mouse_dev, &ev)) // for mouse
    //     input_manager_handle_event(ev.type, ev.code, ev.value);
}

// ----------------------------------------------------
// Main compositor loop (poll input + draw)
// ----------------------------------------------------
void compositor_main(void)
{
    log_info("COMPOSITOR", "Running main loop (input via input_get_mouse)");
    const MouseState* mouseState;
    const KeyboardState* keyboardState;

    int last_x = -1, last_y = -1;


    // // Test Mouse Move
    InputEvent events[8];
    int fd_keyboard = VFS_Open("/dev/input/event0", VFS_FD_STDIN);

    if (fd_keyboard < 0)
    {
        log_error("USR", "Cannot open /dev/input/event0");
    }

    int fd_mouse = VFS_Open("/dev/input/event1", VFS_FD_STDIN);
    if (fd_mouse < 0)
    {
        log_error("USR", "Cannot open /dev/input/event1");
    }


    while (1)
    {

        // Read Mouse Events
        int bytes_mouse = VFS_Read(fd_mouse, events, sizeof(events));
        if (bytes_mouse > 0)
        {
            int noOfMouseEvents = bytes_mouse / sizeof(InputEvent);
            for (int i = 0; i < noOfMouseEvents; i++)
            {
                input_manager_handle_event(events[i].type, events[i].code, events[i].value);
            }
        }

        const MouseState* mouseState = input_get_mouse();
        // log_info("INPUT", "Mouse motion dx=%d dy=%d abs=(%d,%d)", mouseState->dx, mouseState->dy, mouseState->x, mouseState->y);
        if (mouseState->x != last_x || mouseState->y != last_y)
        {
            mouse_x = mouseState->x;
            mouse_y = mouseState->y;

            compositor_redraw();

            last_x = mouse_x;
            last_y = mouse_y;
        }

        const KeyboardState* keyboardState = input_get_keyboard();

        if (keyboardState->keys['a'])
        {
            log_info("INPUT", "Key A pressed");
        }
    }
}