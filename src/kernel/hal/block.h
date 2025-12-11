// block.h
#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct block_device {
    char name[16];
    int (*read_sectors)(struct block_device *dev, uint32_t lba, uint32_t count, void *buf);
    uint32_t lba_base;     // partition start LBA (0 for whole disk, 2048 for first partition, etc.)
    uint32_t sector_size;  // usually 512
    void *private_data;    // controller/driver specific
} block_device_t;

typedef struct {
    uint8_t device;      // 0 = master, 1 = slave
    uint16_t io_base;    // primary = 0x1F0, secondary = 0x170
    uint16_t ctrl_base;  // primary = 0x3F6, secondary = 0x376
} ata_device_t;


#define MAX_BLOCK_DEVICES 8

extern block_device_t *block_devices[MAX_BLOCK_DEVICES];
void block_register(block_device_t *dev);
block_device_t *block_lookup_by_name(const char *name);
// block_device_t* block_lookup_by_lba(uint32_t lba);
