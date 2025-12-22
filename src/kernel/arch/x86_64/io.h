#pragma once
#include <stdint.h>

extern uint8_t x64_inb(uint16_t port);
extern void    x64_outb(uint16_t port, uint8_t val);

extern uint16_t x64_inw(uint16_t port);
extern void     x64_outw(uint16_t port, uint16_t val);

extern void x64_enable_interrupts(void);
extern void x64_disable_interrupts(void);

static inline uint8_t inb(uint16_t port) {
    return x64_inb(port);
}

static inline void outb(uint16_t port, uint8_t val) {
    x64_outb(port, val);
}

static inline uint16_t inw(uint16_t port) {
    return x64_inw(port);
}

static inline void outw(uint16_t port, uint16_t val) {
    x64_outw(port, val);
}


static inline void enable_interrupts(void) {
    x64_enable_interrupts();
}

static inline void disable_interrupts(void) {
    x64_disable_interrupts();
}
