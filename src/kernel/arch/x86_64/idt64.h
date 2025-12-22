// idt64.h
#pragma once
#include <stdint.h>

// Reuse your IDT_FLAGS enum for type/DPL/P bits.
// The gate type values are the same; we just treat them as 64-bit gates.
#include "idt.h"  // for IDT_FLAGS

typedef struct
{
    uint16_t OffsetLow;      // bits 0..15 of handler
    uint16_t SegmentSelector;
    uint8_t  Ist;            // bits 0..2 = IST, bits 3..7 = zero
    uint8_t  Flags;          // type, DPL, P
    uint16_t OffsetMid;      // bits 16..31
    uint32_t OffsetHigh;     // bits 32..63
    uint32_t Reserved;       // must be zero
} __attribute__((packed)) IDTEntry64;

typedef struct
{
    uint16_t    Limit;
    IDTEntry64* Ptr;
} __attribute__((packed)) IDTDescriptor64;

void x64_IDT_Initialize(void);
void x64_IDT_SetGate(int interrupt, void* base, uint16_t segmentSelector, uint8_t flags, uint8_t ist);
