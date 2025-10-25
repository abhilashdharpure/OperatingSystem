#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>
#include <arch/i686/ps2.h>
#include <arch/i686/pit.h>

void HAL_Initialize()
{
    VGA_clrscr();
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    InitializeDevNull();

    // init keyboard
    ps2_init();  

    // init timer
    uint32_t frequency = 10;
    pit_init(frequency);
}
