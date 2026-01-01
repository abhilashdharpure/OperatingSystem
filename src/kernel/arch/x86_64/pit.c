#include "pit.h"
#include <arch/x86_64/irq.h>
#include "arch/x86_64/events/input_event.h"
#include <debug.h>
#include "io.h"

#define PIT_BASE_FREQUENCY 1193182ULL   // Hz (hardware PIT clock)

volatile uint64_t pit_ticks = 0;
static uint32_t pit_frequency = 1000; // default 1000 Hz

// IRQ handler for PIT (IRQ0)
void pit_irq_handler(ISRFrame64* regs)
{
    (void)regs;
    pit_ticks++;
}

// Initialize PIT
void pit_init(uint32_t frequency)
{
    log_info("PIT", "Initializing PIT...");

    if (frequency == 0 || frequency > PIT_BASE_FREQUENCY)
        frequency = 1000; // sane default

    pit_frequency = frequency;
    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    log_info("PIT", "Frequency=%u Hz, divisor=%u", pit_frequency, divisor);

    // Register IRQ0 handler
    x64_IRQ_RegisterHandler(HardwareIRQNo_PIT_Timer, pit_irq_handler);
    x64_IRQ_Unmask(HardwareIRQNo_PIT_Timer);

    // Set PIT to mode 3 (square wave), channel 0, lo/hi byte
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    log_info("PIT", "PIT initialized successfully");
}

// Get number of ticks since PIT init
uint64_t pit_get_ticks(void)
{
    return pit_ticks;
}

// Return system time in microseconds
uint64_t get_system_time_us(void)
{
    uint64_t ticks_snapshot;

    // read atomically
    __asm__ volatile("cli");
    ticks_snapshot = pit_ticks;
    __asm__ volatile("sti");

    return (ticks_snapshot * 1000000ULL) / pit_frequency;
}