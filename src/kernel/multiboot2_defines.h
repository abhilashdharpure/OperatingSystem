#pragma once

#include <stdint.h>

#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8
#define MULTIBOOT_TAG_TYPE_MMAP 6

typedef struct
{
    uint32_t total_size;
    uint32_t reserved;
    uint8_t  tags[];
} multiboot2_info_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
} multiboot2_tag_header_t;

typedef struct
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} multiboot2_mmap_entry_t;


typedef struct
{
    uint32_t type;          // MULTIBOOT_TAG_TYPE_MMAP
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot2_mmap_entry_t entries[];
} multiboot2_tag_mmap_t;


struct multiboot2_mmap_entry
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
};

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint64_t addr;   // physical address
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type_specific[7];
} multiboot2_fb_tag_t;