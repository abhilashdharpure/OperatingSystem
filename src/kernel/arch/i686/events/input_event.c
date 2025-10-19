#include "input_event.h"

void input_device_push_event(InputEvent* inputEvent)
{
    // TODO: update to


    // Here’s what a single input_event might look like in /dev/input/event0 (in hex):

    // 00000000  5e 8a 2b 60 00 00 00 00  56 34 12 00 00 00 00 00
    // 00000010  01 00 1e 00 01 00 00 00


    // Breakdown:

    // 5e 8a 2b 60 ... → timestamp (tv_sec, tv_usec)

    // 01 00 → type = 0x0001 = EV_KEY

    // 1e 00 → code = 0x001e = KEY_A

    // 01 00 00 00 → value = 1 (pressed)

    

    // int fd = open("/dev/input/event0", O_RDONLY);
    // struct input_event ev;

    // while (read(fd, &ev, sizeof(ev)) > 0) {
    //     printf("type=%u code=%u value=%d\n", ev.type, ev.code, ev.value);
    // }
}