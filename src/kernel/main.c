#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "drivers/fb/fb.h"
#include "drivers/fb/fb_graphics.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <boot/bootparams.h>
#include "drivers/input/input.h"
#include "drivers/input/input_manager.h"
#include "compositor/compositor.h"

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
    log_info("Main", "After PS 2 Init!");

end:
    // for (;;);

    compositor_init();
    // Launch compositor
    compositor_main();  // infinite loop

    // // Read mouse and keyboard events from /dev/input/eventX file.
    //test_mouse_keyboard_read();


    // // Test Mouse Move
    // InputEvent events[8];
    // int fd_keyboard = VFS_Open("/dev/input/event0", VFS_FD_STDIN);

    // if (fd_keyboard < 0) {
    //     log_error("USR", "Cannot open /dev/input/event0");
    // }

    // int fd_mouse = VFS_Open("/dev/input/event1", VFS_FD_STDIN);
    // if (fd_mouse < 0) {
    //     log_error("USR", "Cannot open /dev/input/event1");
    // }

    // int32_t x_move = 0;
    // int32_t y_move = 0;


    // while (1)
    // {
    //     // Read Mouse Events
    //     int bytes_mouse = VFS_Read(fd_mouse, events, sizeof(events));
    //     if (bytes_mouse > 0)
    //     {
    //         int noOfMouseEvents = bytes_mouse / sizeof(InputEvent);
    //         for (int i = 0; i < noOfMouseEvents; i++)
    //         {
    //             log_info("Mouse Event", "Reading from file /dev/input/event1: type=%u code=%u value=%d time=%llu",
    //                     events[i].type, events[i].code, events[i].value, events[i].time);

    //             input_manager_handle_event(events[i].type, events[i].code, events[i].value);

                    
    //             if( events[i].type == EV_REL)
    //             {
    //                 if(events[i].code == REL_X)
    //                 {
    //                     x_move += events[i].value;
    //                 }
    //                 else if(events[i].code == REL_Y)
    //                 {
    //                     y_move += events[i].value;
    //                 }
    //                 gfx_fill_rect(x_move, y_move, 5, 5, COLOR_BLUE);  // draw blue cusrsor
    //             }
    //         }
    //     }

    //     // Read Keyboard Events
    //     int bytes_keyboard = VFS_Read(fd_keyboard, events, sizeof(events));
    //     if(bytes_keyboard > 0)
    //     {
    //         int noOfKeyboardEvents = bytes_keyboard / sizeof(InputEvent);
    //         for (int i = 0; i < noOfKeyboardEvents; i++)
    //         {
    //             log_info("Keyboard Event", "Reading from file /dev/input/event0: type=%u code=%u value=%d time=%llu",
    //                     events[i].type, events[i].code, events[i].value, events[i].time);

    //             input_manager_handle_event(events[i].type, events[i].code, events[i].value);
    //         }
    //     }




    //     const MouseState* m = input_get_mouse();
    //     const KeyboardState* k = input_get_keyboard();

    //     // log_info("INPUT", "Mouse: x=%d y=%d L=%d R=%d",
    //     //         m->x, m->y, m->left_button, m->right_button);

    //     if (k->keys['a'])
    //     {
    //         log_info("INPUT", "Key A pressed");
    //     }
            
    // } // While loop
}
