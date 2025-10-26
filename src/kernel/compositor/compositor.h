#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "drivers/fb/fb.h"
#include "drivers/input/input_manager.h"
#include "drivers/input/input.h"

// ------------------------------------------
// Basic compositor configuration
// ------------------------------------------
#define COMPOSITOR_FPS 60      // target frames per second
#define COMPOSITOR_BG_COLOR RGB(0, 0, 0)  // black background

// ------------------------------------------
// Simple struct for a graphical surface/window
// (expand later for Wayland-like behavior)
// ------------------------------------------
typedef struct {
    int x, y;                   // position on screen
    int width, height;           // dimensions
    uint32_t color;              // fill color
    bool visible;
} Surface;

// ------------------------------------------
// Compositor lifecycle functions
// ------------------------------------------

/**
 * Initialize compositor structures and backbuffer.
 */
void compositor_init(void);

/**
 * Main compositor loop — handles input and rendering.
 * For now runs forever in kernel space.
 */
void compositor_main(void);

/**
 * Redraws all visible surfaces.
 */
void compositor_redraw(void);

/**
 * Handles keyboard and mouse input events.
 */
void compositor_handle_input(InputEvent *event);

/**
 * Creates a simple colored rectangle surface.
 */
Surface* compositor_create_surface(int x, int y, int w, int h, uint32_t color);