#include "compositor/compositor.h"
#include "drivers/fb/fb.h"
#include "drivers/input/input_manager.h"
#include <string.h> 
#include "debug.h"

Surface surfaces[MAX_SURFACES];
int surface_count = 0;

// Double-buffer
static uint32_t* backbuffer = NULL;

// Mouse
static int mouse_x, mouse_y;
static int last_mouse_x = -1, last_mouse_y = -1;
static uint32_t cursor_backup[CURSOR_SIZE * CURSOR_SIZE];

// Draw all surfaces into backbuffer
static void draw_surfaces_to_backbuffer(void)
{
    // Clear first
    for (int i = 0; i < fb_dev.width * fb_dev.height; i++)
        backbuffer[i] = COMPOSITOR_BG_COLOR;

    // Draw surfaces
    for (int i = 0; i < surface_count; i++)
    {
        if (!surfaces[i].visible) continue;
        for (int y = 0; y < surfaces[i].height; y++)
        {
            int by = surfaces[i].y + y;
            if (by >= fb_dev.height) break;
            for (int x = 0; x < surfaces[i].width; x++)
            {
                int bx = surfaces[i].x + x;
                if (bx >= fb_dev.width) break;
                backbuffer[by * fb_dev.width + bx] = surfaces[i].color;
            }
        }
    }
}


// Draw a rectangle into any buffer (not necessarily the framebuffer)
static inline void fb_draw_rect_to_buffer(uint32_t* buffer, int buffer_width,
                                          int x, int y, int width, int height,
                                          uint32_t color)
{
    for (int j = 0; j < height; j++)
    {
        int yy = y + j;
        if (yy < 0 || yy >= fb_dev.height)
        {
            continue;
        } 

        for (int i = 0; i < width; i++)
        {
            int xx = x + i;
            if (xx < 0 || xx >= fb_dev.width)
            {
                continue;
            } 

            buffer[yy * buffer_width + xx] = color;
        }
    }
}

// Initialize compositor
void compositor_init(void)
{
    surface_count = 0;
    mouse_x = fb_dev.width / 2;
    mouse_y = fb_dev.height / 2;

    // Allocate backbuffer
    static uint32_t backbuffer_static[1024 * 768];
    backbuffer = backbuffer_static;
    log_info("COMPOSITOR", "Backbuffer using static buffer at %p", backbuffer);
    log_info("COMPOSITOR", "Backbuffer allocated for %dx%d", fb_dev.width, fb_dev.height);

    if (!backbuffer)
    {
        log_error("COMPOSITOR", "Failed to allocate backbuffer!");
        return;
    }
    log_info("COMPOSITOR", "Backbuffer allocated at %p", backbuffer);

        // Clear backbuffer
    for (int i = 0; i < fb_dev.width * fb_dev.height; i++)
    {
        backbuffer[i] = COMPOSITOR_BG_COLOR;
    }

    // Create green surface
    compositor_create_surface(100, 100, 200, 150, RGB(0, 255, 0));

    // Draw all surfaces into backbuffer once
    draw_surfaces_to_backbuffer();

    // Draw surfaces into backbuffer
    for (int i = 0; i < surface_count; i++)
    {
        fb_draw_rect_to_buffer(backbuffer, fb_dev.width,
                               surfaces[i].x, surfaces[i].y,
                               surfaces[i].width, surfaces[i].height,
                               surfaces[i].color);
    }
}

// Add a new surface
void compositor_add_surface(int x, int y, int w, int h, uint32_t color)
{
    if (surface_count >= MAX_SURFACES) return;
    surfaces[surface_count].x = x;
    surfaces[surface_count].y = y;
    surfaces[surface_count].width = w;
    surfaces[surface_count].height = h;
    surfaces[surface_count].color = color;
    surfaces[surface_count].visible = 1;
    surface_count++;
}

Surface* compositor_create_surface(int x, int y, int w, int h, uint32_t color)
{
    if (surface_count >= MAX_SURFACES)
    {
        return NULL;
    }

    Surface* s = &surfaces[surface_count++];
    s->x = x;
    s->y = y;
    s->width = w;
    s->height = h;
    s->color = color;
    s->visible = true;
    s->moving = false;

    log_info("Compositor", "Surface count : %d", surface_count);
    return s;
}

// Draw all surfaces to the backbuffer (does NOT draw the cursor)
void compositor_draw_scene(void)
{
    // Clear backbuffer
    for (int y = 0; y < fb_dev.height; y++)
        for (int x = 0; x < fb_dev.width; x++)
            backbuffer[y * fb_dev.width + x] = COMPOSITOR_BG_COLOR;

    // Draw all visible surfaces
    for (int i = 0; i < surface_count; i++)
    {
        if (surfaces[i].visible)
        {
            fb_draw_rect_to_buffer(backbuffer, fb_dev.width,
                                   surfaces[i].x, surfaces[i].y,
                                   surfaces[i].width, surfaces[i].height,
                                   surfaces[i].color);
        }
    }
}

// Draw cursor only on top of the backbuffer and copy to framebuffer
void compositor_draw_cursor(void)
{
    int cursor_size = CURSOR_SIZE;
    int mx = mouse_x;
    int my = mouse_y;

    // Copy backbuffer to framebuffer
    for (int y = 0; y < fb_dev.height; y++)
    {
        for (int x = 0; x < fb_dev.width; x++)
        {
            ((uint32_t*)fb_dev.framebuffer)[y * fb_dev.width + x] = backbuffer[y * fb_dev.width + x];
        }
    }

    // Draw cursor (blue square)
    for (int y = 0; y < cursor_size; y++)
    {
        int yy = my + y;
        if (yy >= fb_dev.height) break;

        for (int x = 0; x < cursor_size; x++)
        {
            int xx = mx + x;
            if (xx >= fb_dev.width) break;

            ((uint32_t*)fb_dev.framebuffer)[yy * fb_dev.width + xx] = RGB(0, 0, 255);
        }
    }
}

// Redraw framebuffer from backbuffer + cursor
void compositor_redraw(void)
{
    compositor_draw_scene();
    compositor_draw_cursor();
}

void compositor_redraw_cursor_only(void)
{
    // // Restore previous cursor area from backbuffer
    // for (int y = 0; y < CURSOR_SIZE; y++)
    //     for (int x = 0; x < CURSOR_SIZE; x++)
    //     {
    //         int px = last_mouse_x + x;
    //         int py = last_mouse_y + y;
    //         if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
    //             fb_put_pixel(px, py, backbuffer[py * fb_dev.width + px]);
    //     }

    // // Draw cursor at new position
    // for (int y = 0; y < CURSOR_SIZE; y++)
    //     for (int x = 0; x < CURSOR_SIZE; x++)
    //     {
    //         int px = mouse_x + x;
    //         int py = mouse_y + y;
    //         if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
    //             fb_put_pixel(px, py, RGB(0, 0, 255));
    //     }

    // last_mouse_x = mouse_x;
    // last_mouse_y = mouse_y;


        // Copy backbuffer to framebuffer first
    memcpy((void*)fb_dev.framebuffer, backbuffer,
           fb_dev.width * fb_dev.height * sizeof(uint32_t));

    // Draw cursor on top
    for (int y = 0; y < CURSOR_SIZE; y++)
        for (int x = 0; x < CURSOR_SIZE; x++)
        {
            int px = mouse_x + x;
            int py = mouse_y + y;
            if (px < fb_dev.width && py < fb_dev.height)
                ((uint32_t*)fb_dev.framebuffer)[py * fb_dev.width + px] = RGB(0, 0, 255);
        }
}

void compositor_draw_test_window(void)
{
    int win_x = 200;
    int win_y = 150;
    int win_w = 300;
    int win_h = 200;
    uint32_t color = RGB(0, 255, 0); // Green

    for (int y = 0; y < win_h; y++) {
        for (int x = 0; x < win_w; x++) {
            int px = win_x + x;
            int py = win_y + y;
            if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
                ((uint32_t*)fb_dev.framebuffer)[py * fb_dev.width + px] = color;
        }
    }
}

// Main loop
void compositor_main(void)
{
    log_info("COMPOSITOR", "Starting main loop...");

    InputEvent events[8];
    int fd_keyboard = VFS_Open("/dev/input/event0", VFS_FD_STDIN);
    int fd_mouse    = VFS_Open("/dev/input/event1", VFS_FD_STDIN);

    // Just for testing
    // compositor_draw_test_window();
    static Surface* grabbed_surface = NULL;
    static int grab_offset_x = 0, grab_offset_y = 0;


    while (1)
    {
        // 1. Keyboard
        int bytes_kb = VFS_Read(fd_keyboard, events, sizeof(events));
        if (bytes_kb > 0)
        {
            int n = bytes_kb / sizeof(InputEvent);
            for (int i = 0; i < n; i++)
                input_manager_handle_event(events[i].type, events[i].code, events[i].value);
        }

        // 2. Mouse
        int bytes_mouse = VFS_Read(fd_mouse, events, sizeof(events));
        if (bytes_mouse > 0)
        {
            int n = bytes_mouse / sizeof(InputEvent);
            for (int i = 0; i < n; i++)
                input_manager_handle_event(events[i].type, events[i].code, events[i].value);
        }

        // 3. Update mouse
        const MouseState* mouseState = input_get_mouse();

        if (mouseState->x != last_mouse_x || mouseState->y != last_mouse_y)
        {
            // log_info("Compositor", "Mouse Moved!");
            mouse_x = mouseState->x;
            mouse_y = mouseState->y;

            // Only redraw cursor — keeps the green rectangle intact
            // compositor_redraw_cursor_only();

            compositor_redraw();

            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
        }
        if (mouseState->left_button && grabbed_surface) {
            grabbed_surface->x = mouseState->x - grab_offset_x;
            grabbed_surface->y = mouseState->y - grab_offset_y;

            // Redraw backbuffer + cursor
            // 1. Clear backbuffer
            for (int i = 0; i < fb_dev.width * fb_dev.height; i++)
                backbuffer[i] = COMPOSITOR_BG_COLOR;

            // 2. Draw all surfaces
            for (int i = 0; i < surface_count; i++)
                fb_draw_rect_to_buffer(backbuffer, fb_dev.width,
                                    surfaces[i].x, surfaces[i].y,
                                    surfaces[i].width, surfaces[i].height,
                                    surfaces[i].color);

            // 3. Draw cursor on top
            compositor_redraw_cursor_only();
        }
        // On mouse press — check if cursor hits a surface
        else if (mouseState->left_button && !grabbed_surface)
        {
            // log_info("Compositor", "Left button Clicked!");
            for (int i = surface_count - 1; i >= 0; i--) { // topmost first
                Surface* s = &surfaces[i];
                if (mouseState->x >= s->x && mouseState->x < s->x + s->width &&
                    mouseState->y >= s->y && mouseState->y < s->y + s->height)
                {
                    grabbed_surface = s;
                    grab_offset_x = mouseState->x - s->x;
                    grab_offset_y = mouseState->y - s->y;
                    s->moving = true;
                    break;
                }
            }
        }
        // Mouse released
        else if (!mouseState->left_button && grabbed_surface) {
            grabbed_surface->moving = false;
            grabbed_surface = NULL;
        }
        // // Just moving cursor without dragging
        // else if (mouseState->x != last_mouse_x || mouseState->y != last_mouse_y) {
        //     mouse_x = mouseState->x;
        //     mouse_y = mouseState->y;
        //     compositor_redraw_cursor_only();  // only redraw cursor
        //     last_mouse_x = mouse_x;
        //     last_mouse_y = mouse_y;
        // }
    }
}