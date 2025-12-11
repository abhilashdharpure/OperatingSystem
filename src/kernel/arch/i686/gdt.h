#pragma once
#include <stdint.h>

/* literal selectors we will use */
#define KERNEL_CODE_SELECTOR 0x08  /* index 1 << 3 */
#define KERNEL_DATA_SELECTOR 0x10  /* index 2 << 3 */
#define USER_CODE_SELECTOR   0x1B  /* (3 << 3) | 3 */
#define USER_DATA_SELECTOR   0x23  /* (4 << 3) | 3 */


/* Segment selectors */
#define I686_GDT_KERNEL_CODE_SEL   0x08
#define I686_GDT_KERNEL_DATA_SEL   0x10
#define I686_GDT_USER_CODE_SEL     0x1B
#define I686_GDT_USER_DATA_SEL     0x23
#define I686_GDT_TSS_SEL           0x28

// Helper macros
#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)


/* Compatibility with older names used by ISR code */
#define i686_GDT_CODE_SEGMENT  I686_GDT_KERNEL_CODE_SEL
#define i686_GDT_DATA_SEGMENT  I686_GDT_KERNEL_DATA_SEL

// extern GDT_ACCESS;
// typedef struct GDTEntry GDTEntry;


typedef struct __attribute__((packed)) {
    uint16_t LimitLow;     // limit (bits 0-15)
    uint16_t BaseLow;      // base (bits 0-15)
    uint8_t BaseMiddle;    // base (bits 16-23)
    uint8_t Access;        // access
    uint8_t FlagsLimitHi;  // limit (bits 16-19) | flags
    uint8_t BaseHigh;      // base (bits 24-31)
} GDTEntry;

/* GDTR */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} GDTR;

typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

typedef enum 
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;


// typedef struct
// {
//     uint16_t Limit;                     // sizeof(gdt) - 1
//     GDTEntry* Ptr;                      // address of GDT
// } __attribute__((packed)) GDTDescriptor;



// /* GDTR (canonical layout for lgdt) */
// typedef struct __attribute__((packed)) {
//     uint16_t limit;
//     uint32_t base;
// } GDTR;


/* Expose GDT array & size for loader or debugging (optional) */
extern GDTEntry g_GDT[];
extern uint32_t g_GDT_size;


void i686_GDT_Initialize();