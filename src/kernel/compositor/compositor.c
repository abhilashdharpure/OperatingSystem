#include "compositor/compositor.h"
#include "drivers/fb/fb.h"
#include "drivers/input/input_manager.h"
#include <util/string.h> 
#include "debug.h"

static Surface surfaces[MAX_SURFACES];
static int surface_count = 0;

// Double-buffer
static uint32_t* backbuffer = NULL;

// Mouse
static int mouse_x, mouse_y;
static int last_mouse_x = -1, last_mouse_y = -1;
static uint32_t cursor_backup[CURSOR_SIZE * CURSOR_SIZE];

// Initialize compositor
void compositor_init(void)
{
    surface_count = 0;
    mouse_x = fb_dev.width / 2;
    mouse_y = fb_dev.height / 2;

    // Allocate backbuffer
    // backbuffer = kmalloc(fb_dev.width * fb_dev.height * sizeof(uint32_t));
    static uint32_t backbuffer_static[1024 * 768];
    backbuffer = backbuffer_static;
    log_info("COMPOSITOR", "Backbuffer using static buffer at %p", backbuffer);
    log_info("COMPOSITOR", "Backbuffer allocated for %dx%d", fb_dev.width, fb_dev.height);

    if (!backbuffer) {
        log_error("COMPOSITOR", "Failed to allocate backbuffer!");
        return;
    }
    log_info("COMPOSITOR", "Backbuffer allocated at %p", backbuffer);

        // Clear backbuffer
    for (int i = 0; i < fb_dev.width * fb_dev.height; i++)
        backbuffer[i] = COMPOSITOR_BG_COLOR;
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

// Redraw framebuffer from backbuffer + cursor
void compositor_redraw(void)
{
    // Fill the backbuffer with background
    for (int y = 0; y < fb_dev.height; y++) {
        for (int x = 0; x < fb_dev.width; x++) {
            backbuffer[y * fb_dev.width + x] = COMPOSITOR_BG_COLOR;
        }
    }

    // Draw all visible surfaces (example)
    for (int i = 0; i < surface_count; i++) {
        if (surfaces[i].visible) {
            for (int y = 0; y < surfaces[i].height; y++) {
                for (int x = 0; x < surfaces[i].width; x++) {
                    int dst_x = surfaces[i].x + x;
                    int dst_y = surfaces[i].y + y;
                    if (dst_x >= 0 && dst_x < fb_dev.width &&
                        dst_y >= 0 && dst_y < fb_dev.height)
                    {
                        backbuffer[dst_y * fb_dev.width + dst_x] = surfaces[i].color;
                    }
                }
            }
        }
    }

    // Draw mouse cursor (blue square)
    for (int y = 0; y < CURSOR_SIZE; y++) {
        for (int x = 0; x < CURSOR_SIZE; x++) {
            int dst_x = mouse_x + x;
            int dst_y = mouse_y + y;
            if (dst_x >= 0 && dst_x < fb_dev.width &&
                dst_y >= 0 && dst_y < fb_dev.height)
            {
                backbuffer[dst_y * fb_dev.width + dst_x] = RGB(0, 0, 255);
            }
        }
    }

    // Copy backbuffer → physical framebuffer
    uint32_t* fb = (uint32_t*)fb_dev.framebuffer;   // make sure this field name matches your fb struct
    uint32_t pixels = fb_dev.width * fb_dev.height;
    for (uint32_t i = 0; i < pixels; i++) {
        fb[i] = backbuffer[i];
    }
}

void compositor_redraw_cursor_only(void)
{
    // Restore background under old cursor
    if (last_mouse_x >= 0 && last_mouse_y >= 0)
    {
        for (int y = 0; y < CURSOR_SIZE; y++)
        {
            for (int x = 0; x < CURSOR_SIZE; x++)
            {
                int px = last_mouse_x + x;
                int py = last_mouse_y + y;
                if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
                {
                    uint32_t color = cursor_backup[y * CURSOR_SIZE + x];
                    fb_put_pixel(px, py, color);
                }
            }
        }
    }

    // Save new background
    for (int y = 0; y < CURSOR_SIZE; y++)
    {
        for (int x = 0; x < CURSOR_SIZE; x++)
        {
            int px = mouse_x + x;
            int py = mouse_y + y;
            if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
            {
                cursor_backup[y * CURSOR_SIZE + x] =
                    ((uint32_t*)fb_dev.framebuffer)[py * fb_dev.width + px];
            }
        }
    }

    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    // Draw the cursor itself (blue block)
    for (int y = 0; y < CURSOR_SIZE; y++)
    {
        for (int x = 0; x < CURSOR_SIZE; x++)
        {
            int px = mouse_x + x;
            int py = mouse_y + y;
            if (px >= 0 && px < fb_dev.width && py >= 0 && py < fb_dev.height)
                fb_put_pixel(px, py, RGB(0, 0, 255));
        }
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
    compositor_draw_test_window();
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
        // if (mouseState->x != mouse_x || mouseState->y != mouse_y)
        // {
        //     mouse_x = mouseState->x;
        //     mouse_y = mouseState->y;

        //     // Clamp
        //     if (mouse_x < 0) mouse_x = 0;
        //     if (mouse_y < 0) mouse_y = 0;
        //     if (mouse_x >= fb_dev.width - CURSOR_SIZE)  mouse_x = fb_dev.width - CURSOR_SIZE;
        //     if (mouse_y >= fb_dev.height - CURSOR_SIZE) mouse_y = fb_dev.height - CURSOR_SIZE;

        //     compositor_redraw();
        // }
        if (mouseState->x != last_mouse_x || mouseState->y != last_mouse_y)
{
            mouse_x = mouseState->x;
            mouse_y = mouseState->y;
            compositor_redraw_cursor_only();
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
        }
    }
}