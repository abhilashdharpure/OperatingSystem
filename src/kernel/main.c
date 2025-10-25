#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "drivers/fb/fb.h"
#include "drivers/fb/fb_graphics.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <boot/bootparams.h>

extern void _init();

void crash_me();

void timer(Registers* regs)
{
    printf(".");
}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    // call global constructors
    _init();

    HAL_Initialize();

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }


    log_info("Main", "This is an info msg!");
    log_warning("Main", "This is a warning msg!");
    log_error("Main", "This is an error msg!");
    log_critical("Main", "This is a critical msg!");
    printf("Welcome to One OS v0.1\n");
    printf("This operating system is under construction.\n");

    //crash_me();


    // initialize framebuffer
    fb_init(fb_info);
    gfx_init();            // initialize graphics layer
    //test_fb();


    gfx_clear(COLOR_BLACK);              // clear screen
    gfx_fill_rect(50, 50, 400, 200, COLOR_GREEN);  // draw green rectangle
    gfx_put_pixel(10, 10, COLOR_RED);   // draw red pixel

    gfx_draw_hline(20, 200, 500, COLOR_BLUE );
    gfx_draw_vline(200, 20, 500, COLOR_RED );
    log_info("Main", "After PS 2 Init!");

end:
    for (;;);

    // // Read mouse and keyboard events from /dev/input/eventX file.
    //test_mouse_keyboard_read();
}
