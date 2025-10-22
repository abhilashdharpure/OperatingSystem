#pragma once

#include <stdint.h>
#include "arch/i686/sys/time.h"

// typedef struct  {
//     uint64_t time;        // when the event happened // microseconds since boot (optional)
//     uint16_t type;        // EV_KEY, EV_REL, etc.
//     uint16_t code;        // KEY_A, REL_X, BTN_LEFT, etc.
//     int32_t  value;       // meaning depends on type+code
// } InputEvent;


enum InputEventType{
    EV_SYN = 0,	    //Sync event (frame separator)
	EV_KEY = 1,	    //Key or button press/release
	EV_REL = 2,	    //Relative movement (e.g. mouse)
	EV_ABS = 3,	    //Absolute position (e.g. touch)
	EV_MSC = 4,	    //Misc (e.g., scancode)
	EV_LED = 17,	//LED state (CapsLock, NumLock)
	EV_REP = 20	    //Repeat rate
} ;

enum KeyboardKeyValue{
    KEY_RELEASED        = 0,
    KEY_PRESSED         = 1,
    KEY_AUTO_REPEAT     = 2
};

enum MouseEventValue{
    REL_X         = 0x00,
    REL_Y         = 0x01,
    REL_WHEEL     = 0x08
};

// IRQ	    Purpose
// 0–7	    Handled by PIC1 (Master)
// 8–15	    Handled by PIC2 (Slave)
// 2	    Cascade from PIC1 to PIC2
enum HardwareIRQNumber
{
    HardwareIRQNo_PIT_Timer     = 0,
    HardwareIRQNo_Keyboard      = 1,
    HardwareIRQNo_Cascade       = 2,
    HardwareIRQNo_Mouse         = 12
};