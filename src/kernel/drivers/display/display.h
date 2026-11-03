#pragma once

#include "stdint.h"
#include <string.h>
#include "drivers/fb/fb.h"


// FOr Framebuffer
#define KERNEL_FB_VA 0xFFFFFFFFC0000000ULL


extern fb_device_t fb_dev;

void display_devices_init(void);
