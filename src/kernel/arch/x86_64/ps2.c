#include "arch/x86_64/ps2.h"
#include <arch/x86_64/irq.h>
#include <stdbool.h>
#include <stdint.h>
#include <debug.h>
#include <string.h>
#include "time/time.h"

#include "arch/x86_64/events/input_event.h"
#include "drivers/input/input.h"

#include "hal/vfs.h"
#include "io.h"

#define DATA_PORT 0x60
#define STATUS_PORT 0x64


/* ---- keyboard-driven pointer: F8 toggles; arrows move, Enter = left click, Right Ctrl = right click ---- */
#define KP_F8      66
#define KP_UP     103
#define KP_DOWN   108
#define KP_LEFT   105
#define KP_RIGHT  106
#define KP_ENTER   28
#define KP_KPENTER 96
#define KP_RCTRL   97

static bool    kp_mode;
static uint8_t kp_rep[4];                    /* auto-repeat counts: up, down, left, right */
static bool    kp_left_down, kp_right_down;

// Simple key buffer
#define KEY_BUFFER_SIZE 32
#define SYN_REPORT 0
static volatile char key_buffer[KEY_BUFFER_SIZE];
static volatile int key_head = 0;
static volatile int key_tail = 0;

static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];
static uint8_t prev_buttons = 0;

struct input_device keyboard_dev;
struct input_device mouse_dev;

static bool e0_pending = false;

static uint16_t e0_to_evdev(uint8_t make)
{
    switch (make) {
        case 0x1C: return 96;   // KP Enter
        case 0x1D: return 97;   // Right Ctrl
        case 0x35: return 98;   // KP /
        case 0x38: return 100;  // Right Alt
        case 0x47: return 102;  // Home
        case 0x48: return 103;  // Up
        case 0x49: return 104;  // Page Up
        case 0x4B: return 105;  // Left
        case 0x4D: return 106;  // Right
        case 0x4F: return 107;  // End
        case 0x50: return 108;  // Down
        case 0x51: return 109;  // Page Down
        case 0x52: return 110;  // Insert
        case 0x53: return 111;  // Delete
        default:   return 0;
    }
}


static bool kbc_wait_in(void)  { for (int i = 0; i < 100000; i++) if (!(inb(STATUS_PORT) & 2)) return true; return false; }
static bool kbc_wait_out(void) { for (int i = 0; i < 100000; i++) if (inb(STATUS_PORT) & 1)   return true; return false; }

static void kp_push(uint16_t type, uint16_t code, int32_t value)
{
    InputEvent ev = {0};
    ev.time.tv_sec = get_system_time();
    ev.type = type; ev.code = code; ev.value = value;
    input_push_event(&mouse_dev, &ev);
}

static void kp_led(bool on)                  /* the Caps Lock LED shows pointer mode; harmless if it doesn't light */
{
    if (!kbc_wait_in()) return;
    outb(DATA_PORT, 0xED);
    if (kbc_wait_out()) (void)inb(DATA_PORT);               /* ACK */
    if (!kbc_wait_in()) return;
    outb(DATA_PORT, on ? 0x04 : 0x00);
    if (kbc_wait_out()) (void)inb(DATA_PORT);               /* ACK */
}

static int kp_step(uint8_t reps) { return reps < 3 ? 6 : (reps < 10 ? 14 : 28); }   /* accelerates while a key repeats */

static bool kp_filter(uint16_t code, bool released)          /* true = key consumed */
{
    if (code == KP_F8) {
        if (!released) {
            kp_mode = !kp_mode;
            memset(kp_rep, 0, sizeof kp_rep);
            if (!kp_mode) {                                  /* never leave a button stuck down */
                if (kp_left_down)  { kp_push(EV_KEY, BTN_LEFT,  KEY_RELEASED); kp_left_down  = false; }
                if (kp_right_down) { kp_push(EV_KEY, BTN_RIGHT, KEY_RELEASED); kp_right_down = false; }
                kp_push(EV_SYN, SYN_REPORT, 0);
            }
            kp_led(kp_mode);
        }
        return true;
    }
    if (!kp_mode) return false;

    int dir = code == KP_UP ? 0 : code == KP_DOWN ? 1 : code == KP_LEFT ? 2 : code == KP_RIGHT ? 3 : -1;
    if (dir >= 0) {
        if (released) { kp_rep[dir] = 0; return true; }
        int s = kp_step(kp_rep[dir]);
        if (kp_rep[dir] < 255) kp_rep[dir]++;
        kp_push(EV_REL, dir < 2 ? REL_Y : REL_X, (dir == 0 || dir == 2) ? -s : s);   /* +Y is down, as in your mouse handler */
        kp_push(EV_SYN, SYN_REPORT, 0);
        return true;
    }
    if (code == KP_ENTER || code == KP_KPENTER) {
        bool down = !released;
        if (down != kp_left_down) {                           /* ignore typematic repeats */
            kp_left_down = down;
            kp_push(EV_KEY, BTN_LEFT, down ? KEY_PRESSED : KEY_RELEASED);
            kp_push(EV_SYN, SYN_REPORT, 0);
        }
        return true;
    }
    if (code == KP_RCTRL) {
        bool down = !released;
        if (down != kp_right_down) {
            kp_right_down = down;
            kp_push(EV_KEY, BTN_RIGHT, down ? KEY_PRESSED : KEY_RELEASED);
            kp_push(EV_SYN, SYN_REPORT, 0);
        }
        return true;
    }
    return false;
}

// IRQ1 handler
static void ps2_keyboard_handler()
{
    uint8_t sc = inb(DATA_PORT);

    if (sc == 0xE0) {                 // extended prefix: the next byte is the key
        e0_pending = true;
        x64_IRQ_SendEndOfInterupt(HardwareIRQNo_Keyboard);
        return;
    }
    if (sc == 0xE1) {                 // Pause key sequence: ignore
        x64_IRQ_SendEndOfInterupt(HardwareIRQNo_Keyboard);
        return;
    }

    bool     released = sc & 0x80;
    uint8_t  make     = sc & 0x7F;
    uint16_t code     = e0_pending ? e0_to_evdev(make)
                                   : (make <= 0x58 ? make : 0);
    e0_pending = false;

    if (code && kp_filter(code, released)) {
        x64_IRQ_SendEndOfInterupt(HardwareIRQNo_Keyboard);
        return;
    }

    if (code) {
        InputEvent ev = {0};
        ev.time.tv_sec  = get_system_time(); 
        ev.type  = EV_KEY;
        ev.code  = code;
        ev.value = released ? KEY_RELEASED : KEY_PRESSED;
        input_push_event(&keyboard_dev, &ev);

        InputEvent syn = {0};
        syn.time.tv_sec  = get_system_time(); 
        syn.type = EV_SYN;
        syn.code = SYN_REPORT;
        input_push_event(&keyboard_dev, &syn);
    }

    x64_IRQ_SendEndOfInterupt(HardwareIRQNo_Keyboard);   // on every path
}

// IRQ12 handler
static void ps2_mouse_handler(ISR64Handler* regs)
{
    uint8_t data = inb(DATA_PORT);

    // --- Sync to the start of a packet ---
    if (mouse_cycle == 0 && !(data & 0x08)) {
        // Bit3 not set → not a valid first byte, skip
        return;
    }

    mouse_packet[mouse_cycle] = data;
    mouse_cycle++;

    // Wait until we have a full 3-byte packet
    if (mouse_cycle < 3)
    {
        return;
    }
    uint8_t b0 = (uint8_t)mouse_packet[0];
    int8_t x_move = (int8_t)mouse_packet[1] - ((b0 << 4) & 0x100);   // bit 4 = X sign
    int8_t y_move = (int8_t)mouse_packet[2]- ((b0 << 3) & 0x100);   // bit 5 = Y sign

    if (b0 & 0xC0)
    { 
        /* overflow: drop this packet */ 
        mouse_cycle = 0;
        //send EOI;
        return;
    }
    y_move = -y_move;

    bool left_pressed   = mouse_packet[0] & 0x01;   // (0000 0001)
    bool right_pressed  = mouse_packet[0] & 0x02;   // (0000 0010)
    bool middle_pressed = mouse_packet[0] & 0x04;   // (0000 0100)
    uint8_t buttons = mouse_packet[0] & 0x07;       // (0000 0111) bits 0–2 = L/M/R 


    mouse_cycle = 0; // reset for next packet

    // Log or push events
    // log_info("MOUSE", "X=%d Y=%d L=%d R=%d M=%d", x_move, y_move, left_pressed, right_pressed, middle_pressed);

    // (You might want to store previous button state to detect edges)
    InputEvent event;

    // Left button
    if ((prev_buttons & 0x01) != (buttons & 0x01))
    {
        event.time.tv_sec =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_LEFT;
        event.value = left_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);
        // log_info("Mouse", "LEFT button....");
    }

    // Right button
    if ((prev_buttons & 0x02) != (buttons & 0x02))
    {
        event.time.tv_sec =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_RIGHT;
        event.value = right_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);
    }

    // Middle button
    if ((prev_buttons & 0x04) != (buttons & 0x04))
    {
        event.time.tv_sec =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_MIDDLE;
        event.value = middle_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);
    }

    // Movement events
    if (x_move != 0) {
        event.time.tv_sec =  get_system_time();
        event.type = EV_REL;
        event.code = REL_X;
        event.value = x_move;
        input_push_event(&mouse_dev, &event);
    }
    if (y_move != 0) {
        event.time.tv_sec =  get_system_time();
        event.type = EV_REL;
        event.code = REL_Y;
        event.value = y_move;
        input_push_event(&mouse_dev, &event);
    }

    prev_buttons = buttons;

    // Always send sync event at end
    InputEvent synEvent;
    synEvent.time.tv_sec =  get_system_time();
    synEvent.type = EV_SYN;
    synEvent.code = SYN_REPORT;
    synEvent.value = 0;
    input_push_event(&mouse_dev, &synEvent);

    x64_IRQ_SendEndOfInterupt(HardwareIRQNo_Mouse); // Or the proper IRQ number for mouse IRQ12
}

void ps2_test_mouse_polling()
{
    log_info("Mouse", "Polling mouse data for 5 seconds...");

    uint8_t status = inb(STATUS_PORT);
    if ((status & 0x01) && (status & 0x20)) {
        uint8_t byte = inb(DATA_PORT);
        log_info("Mouse", "Byte from mouse: 0x%02x", byte);
    }
}

void ps2_configure_controller_command_byte(void)
{
    if (!kbc_wait_in()) return;
    outb(0x64, 0x20);
    if (!kbc_wait_out()) { log_info("PS/2", "no command byte: no i8042?"); return; }   /* don't write garbage back */
    uint8_t cmd = inb(0x60);
    cmd |= (1 << 1);                  /* IRQ12 on */
    cmd &= ~(1 << 5);                 /* mouse clock on */
    if (!kbc_wait_in()) return;
    outb(0x64, 0x60);
    if (!kbc_wait_in()) return;
    outb(0x60, cmd);
}

bool ps2_mouse_enable_data_reporting() {
    // Step 1: Tell the controller the next byte is for the mouse
    outb(0x64, 0xD4);

    // Step 2: Send 0xF4 to enable data reporting
    outb(0x60, 0xF4);

    // Step 3: Wait for ACK (0xFA)
    for (int i = 0; i < 10000; i++) {
        if (inb(0x64) & 0x01) { // Output buffer full
            uint8_t response = inb(0x60);
            return response == 0xFA; // ACK
        }
    }

    return false; // Timeout or wrong response
}


void ps2_enable_mouse()
{
    // 1. Enable the second PS/2 port (mouse)
    outb(0x64, 0xA8);

    // 2. Enable IRQ12 and mouse clock in controller command byte
    ps2_configure_controller_command_byte();

    // 3. Enable mouse data reporting
    if (!ps2_mouse_enable_data_reporting())
    {
        log_info("Mouse", "Didn't receive ACK after enabling mouse!");
    } 
    else
    {
        log_info("Mouse", "Mouse enabled successfully");
    }

    for (int i = 0; i < 1000; ++i)
    {
        if (inb(0x64) & 0x01) {
            uint8_t dummy = inb(0x60);
            log_info("Mouse", "Unsticking with dummy read: 0x%02x", dummy);
        }
    }
}

void init_input_system(void)
{
    register_input_device(&keyboard_dev, "event0");
    register_input_device(&mouse_dev, "event1");
}

void ps2_init() 
{
    x64_IRQ_RegisterHandler(HardwareIRQNo_Keyboard, ps2_keyboard_handler);
    x64_IRQ_RegisterHandler(HardwareIRQNo_Mouse, ps2_mouse_handler);

    log_info("PS/2", "Registered IRQ handlers");

    ps2_enable_mouse();
    // enable the mouse
    ps2_test_mouse_polling();

    init_input_system();

    log_info("PS/2", "Enable interrupts..");


    // 4. Enable interrupts
    // __asm__ __volatile__("sti");
    log_info("PS/2", "After Enable interrupts..");

    log_info("PS/2", "Keyboard & Mouse initialization complete");
}

void test_mouse_keyboard_read(void)
{
    InputEvent events[8];
    int fd_keyboard = VFS_Open("/dev/input/event0", VFS_FD_STDIN);

    if (fd_keyboard < 0) {
        log_error("USR", "Cannot open /dev/input/event0");
    }

    int fd_mouse = VFS_Open("/dev/input/event1", VFS_FD_STDIN);
    if (fd_mouse < 0) {
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
                log_info("Mouse Event", "Reading from file /dev/input/event1: type=%u code=%u value=%d time=%llu",
                        events[i].type, events[i].code, events[i].value, events[i].time);

                // input_manager_handle_event(events[i].type, events[i].code, events[i].value);
            }
        }

        // Read Keyboard Events
        int bytes_keyboard = VFS_Read(fd_keyboard, events, sizeof(events));
        if(bytes_keyboard > 0)
        {
            int noOfKeyboardEvents = bytes_keyboard / sizeof(InputEvent);
            for (int i = 0; i < noOfKeyboardEvents; i++)
            {
                log_info("Keyboard Event", "Reading from file /dev/input/event0: type=%u code=%u value=%d time=%llu",
                        events[i].type, events[i].code, events[i].value, events[i].time);

                // input_manager_handle_event(events[i].type, events[i].code, events[i].value);
            }
        }
    }
}
