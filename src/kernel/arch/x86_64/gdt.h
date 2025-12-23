#pragma once
#include <stdint.h>

/* Selectors */
#define GDT_KERNEL_CODE 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x1B
#define USER_DATA_SELECTOR   0x23
#define TSS_SELECTOR         0x28

/* GDT entry (8 bytes) */
typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags;
    uint8_t  base_high;
} GDTEntry;

/* TSS descriptor (16 bytes in x86_64) */
typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} TSSDescriptor;

/* GDTR */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} GDTR;

/* Access flags */
#define GDT_ACCESS_PRESENT   0x80
#define GDT_ACCESS_RING0     0x00
#define GDT_ACCESS_RING3     0x60
#define GDT_ACCESS_CODE      0x18
#define GDT_ACCESS_DATA      0x10
#define GDT_ACCESS_RW        0x02

/* Flags */
#define GDT_FLAG_LONG_MODE   0x20
#define GDT_FLAG_GRAN_4K     0x80

void gdt_init(void);
