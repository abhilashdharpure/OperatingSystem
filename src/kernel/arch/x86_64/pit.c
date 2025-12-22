#include "pit.h"
#include <arch/x86_64/irq.h>
#include "arch/x86_64/events/input_event.h"
#include <debug.h>
#include "io.h"

#define PIT_BASE_FREQUENCY 1193182ULL   // Hz (hardware clock rate)

volatile uint64_t pit_ticks = 0;
static uint32_t pit_frequency = 1000;   // default to 1000 Hz (1 ms per tick)

void pit_irq_handler(Registers* regs)
{
    pit_ticks++;
    i686_IRQ_SendEndOfInterupt(HardwareIRQNo_PIT_Timer);
}

void pit_init(uint32_t frequency)
{
    if (frequency == 0 || frequency > PIT_BASE_FREQUENCY)
        frequency = 1000; // sane default

    pit_frequency = frequency;

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    // Command byte: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    i686_IRQ_RegisterHandler(HardwareIRQNo_PIT_Timer, pit_irq_handler);

    log_info("PIT", "Initialized at %u Hz (divisor=%u)", pit_frequency, divisor);
}

uint64_t get_system_time_us(void)
{
    // Convert ticks to microseconds using integer arithmetic
    // Formula: time_us = (pit_ticks * 1_000_000) / pit_frequency

    uint64_t ticks_snapshot;

    // Read atomically (avoid reading while incremented in IRQ)
    __asm__ volatile("cli");
    ticks_snapshot = pit_ticks;
    __asm__ volatile("sti");

    return (ticks_snapshot * 1000000ULL) / pit_frequency;
}

uint64_t pit_get_ticks(void)
{
    uint64_t t;
    __asm__ volatile("cli"); // disable interrupts
    t = pit_ticks;
    __asm__ volatile("sti"); // enable interrupts
    return t;
}

// #define PIT_FREQUENCY     1193182

// volatile uint64_t pit_ticks = 0;

// void pit_irq_handler(Registers* regs)
// {
//     //log_info("PIT", "pit_irq_handler, pit_ticks = %d", pit_ticks);

//     pit_ticks++;
//     i686_IRQ_SendEndOfInterupt(HardwareIRQNo_PIT_Timer);
// }

// void pit_init(uint32_t frequency)
// {
//     log_info("PIT", "pit_init");

//     // uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);
//     // outb(0x43, 0x36);
//     // outb(0x40, divisor & 0xFF);
//     // outb(0x40, divisor >> 8);
//     // // register_interrupt_handler(32, pit_irq_handler);  // IRQ0 vector

//     i686_IRQ_RegisterHandler(HardwareIRQNo_PIT_Timer, pit_irq_handler);

// }

// uint64_t get_system_time_us(void) 
// {
//     // time = (ticks * period) + partial progress of current tick
//     double tick_duration_us = 1e6 / PIT_FREQUENCY;
//     return (uint64_t)(pit_ticks * tick_duration_us);

//     // uint32_t low, high;
//     // __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
//     // return ((uint64_t)high << 32) | low;
// }