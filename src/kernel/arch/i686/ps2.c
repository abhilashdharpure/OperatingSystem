#include "arch/i686/ps2.h"
#include <arch/i686/irq.h>
#include <stdbool.h>
#include <stdint.h>
#include <debug.h>
#include "time/time.h"

#include "arch/i686/events/input_event.h"
#include "drivers/input/input.h"
#include "drivers/input/register_input.h"

#define DATA_PORT 0x60
#define STATUS_PORT 0x64

#define BTN_LEFT    0x110
#define BTN_RIGHT   0x111
#define BTN_MIDDLE  0x112

// Simple key buffer
#define KEY_BUFFER_SIZE 32
#define SYN_REPORT 0
static volatile char key_buffer[KEY_BUFFER_SIZE];
static volatile int key_head = 0;
static volatile int key_tail = 0;

static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];


struct input_device keyboard_dev;
struct input_device mouse_dev;



// Basic US QWERTY Scan Code Set 1 mapping for keys 0x01-0x3A (partial)
static const char scancode_to_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6',  // 0x00-0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t', // 0x08-0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10-0x17
    'o', 'p', '[', ']', '\n', 0, 'a', 's',   // 0x18-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x20-0x27
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',  // 0x28-0x2F
    'b', 'n', 'm', ',', '.', '/', 0, '*',    // 0x30-0x37
    0, ' ', 0, 0, 0, 0, 0, 0,                 // 0x38-0x3F
    // rest zero
};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// IRQ1 handler
static void ps2_keyboard_handler()
{
    uint8_t scancode = inb(DATA_PORT);
    bool released = (scancode & 0x80);
    uint8_t keycode = scancode & 0x7F;

    // Translate scancode → Linux KEY_* code
    uint16_t linux_keycode = scancode_to_ascii[keycode]; // You need this table!

    if (linux_keycode == 0)
        return; // Unknown key

    InputEvent keyEvent;
    keyEvent.time = get_system_time();
    keyEvent.type = EV_KEY;
    keyEvent.code = linux_keycode;
    keyEvent.value = released ? KEY_RELEASED : KEY_PRESSED;


    log_info("PS2", "[Input] type=%u, code=%u, value=%d, time=%llu\n",
                keyEvent.type, keyEvent.code, keyEvent.value, keyEvent.time);
    input_push_event(&keyboard_dev, &keyEvent); // Your ring buffer writer


    // // Only for testing
    // debug_poll_input(&keyboard_dev);

    if(keyEvent.value == KEY_PRESSED)
    {
        log_info("PS2", "Key Pressed!, button = %c", linux_keycode);
    }
    else
    {
        log_info("PS2", "Key Released!, button = %c", linux_keycode); 
    }

    // Always follow up with EV_SYN
    InputEvent synEvent;
    synEvent.time =  get_system_time();
    synEvent.type = EV_SYN;
    synEvent.code = SYN_REPORT;
    synEvent.value = 0;

    input_push_event(&keyboard_dev, &synEvent);
    
    i686_IRQ_SendEndOfInterupt(HardwareIRQNo_Keyboard);
}

// IRQ12 handler
static void ps2_mouse_handler(Registers* regs)
{
    log_info("Mouse Event", "Move Event ");

    uint8_t data = inb(DATA_PORT);


    mouse_packet[mouse_cycle] = data;
    mouse_cycle++;

    if (mouse_cycle == 3)
    {
        mouse_cycle = 0;

        bool left_pressed = mouse_packet[0] & 0x01;
        bool right_pressed = mouse_packet[0] & 0x02;
        bool middle_pressed = mouse_packet[0] & 0x04;

        int8_t x_move = mouse_packet[1];
        int8_t y_move = mouse_packet[2];

        // Convert y_move: mouse Y is typically negative for "up"
        y_move = -y_move;

        // Example: push button events if state changed
        // (You might want to store previous button state to detect edges)
        InputEvent event;

        // Left button
        event.time =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_LEFT;
        event.value = left_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);

        // Right button
        event.time =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_RIGHT;
        event.value = right_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);

        // Middle button
        event.time =  get_system_time();
        event.type = EV_KEY;
        event.code = BTN_MIDDLE;
        event.value = middle_pressed ? KEY_PRESSED : KEY_RELEASED;
        input_push_event(&mouse_dev, &event);

        // Movement events
        if (x_move != 0) {
            event.time =  get_system_time();
            event.type = EV_REL;
            event.code = REL_X;
            event.value = x_move;
            input_push_event(&mouse_dev, &event);
        }
        if (y_move != 0) {
            event.time =  get_system_time();
            event.type = EV_REL;
            event.code = REL_Y;
            event.value = y_move;
            input_push_event(&mouse_dev, &event);
        }

        // Always send sync event at end
        InputEvent synEvent;
        event.time =  get_system_time();
        synEvent.type = EV_SYN;
        synEvent.code = SYN_REPORT;
        synEvent.value = 0;
        input_push_event(&mouse_dev, &synEvent);

        // Optional logging
        log_info("Mouse", "Move X=%d Y=%d L=%d R=%d M=%d", x_move, y_move, left_pressed, right_pressed, middle_pressed);

        // // Only for testing
        // debug_poll_input(&mouse_dev);
    }

    i686_IRQ_SendEndOfInterupt(HardwareIRQNo_Mouse); // Or the proper IRQ number for mouse IRQ12
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

void ps2_configure_controller_command_byte() {
    // Request controller to send us the current command byte
    outb(0x64, 0x20);         // Command: Read Controller Command Byte
    uint8_t cmd = inb(0x60);  // Read current command byte

    // Modify it:
    cmd |= (1 << 1);          // Enable IRQ12 (mouse interrupt)
    cmd &= ~(1 << 5);         // Enable mouse clock (clear disable bit)

    // Write modified command byte back
    outb(0x64, 0x60);         // Command: Write Controller Command Byte
    outb(0x60, cmd);          // Send the new command byte
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

// void keyboard_isr(uint8_t scancode, int pressed)
// {
//     uint16_t keycode = translate_scancode(scancode); // implement simple map
//     input_push_event(&keyboard_dev, EV_KEY, keycode, pressed ? 1 : 0);
// }

// void mouse_move(int dx, int dy)
// {
//     input_push_event(&mouse_dev, EV_REL, REL_X, dx);
//     input_push_event(&mouse_dev, EV_REL, REL_Y, dy);
// }

void init_input_system(void)
{
    register_input_device(&keyboard_dev, "event0");
    register_input_device(&mouse_dev, "event1");
}

void ps2_init() 
{
    i686_IRQ_RegisterHandler(HardwareIRQNo_Keyboard, ps2_keyboard_handler);
    i686_IRQ_RegisterHandler(HardwareIRQNo_Mouse, ps2_mouse_handler);

    log_info("PS/2", "Registered IRQ handlers");

    ps2_enable_mouse(); // 👈 Important: enable the mouse
    ps2_test_mouse_polling();

    init_input_system();

    // 4. Enable interrupts
    __asm__ __volatile__("sti");
    log_info("PS/2", "Keyboard & Mouse initialization complete");
}
