#include "pit.h"
#include <arch/i686/irq.h>
#include "arch/i686/events/input_event.h"
#include <debug.h>


#define PIT_FREQUENCY     1193182

volatile uint64_t pit_ticks = 0;

void pit_irq_handler(Registers* regs)
{
    //log_info("PIT", "pit_irq_handler, pit_ticks = %d", pit_ticks);

    pit_ticks++;
    i686_IRQ_SendEndOfInterupt(HardwareIRQNo_PIT_Timer);
}

void pit_init(uint32_t frequency)
{
    log_info("PIT", "pit_init");

    // uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);
    // outb(0x43, 0x36);
    // outb(0x40, divisor & 0xFF);
    // outb(0x40, divisor >> 8);
    // // register_interrupt_handler(32, pit_irq_handler);  // IRQ0 vector

    i686_IRQ_RegisterHandler(HardwareIRQNo_PIT_Timer, pit_irq_handler);

}

uint64_t get_system_time_us(void) 
{
    // time = (ticks * period) + partial progress of current tick
    double tick_duration_us = 1e6 / PIT_FREQUENCY;
    return (uint64_t)(pit_ticks * tick_duration_us);
}