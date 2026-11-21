#pragma once
#include <stdint.h>
#include "arch/x86_64/xhci.h"


#ifndef inl
static inline void outl(uint16_t p, uint32_t v) { __asm__ volatile("outl %0,%1" :: "a"(v), "Nd"(p)); }
static inline uint32_t inl(uint16_t p) { uint32_t v; __asm__ volatile("inl %1,%0" : "=a"(v) : "Nd"(p)); return v; }
#endif

int pci_find(uint16_t vendor, uint16_t device, pci_dev_t *out);
