#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "block.h"

typedef struct {
    block_device_t *dev;    // underlying whole disk or partition device
    uint32_t lba_base;      // offset of this partition (0 for whole disk)
    uint32_t sector_size;   // usually 512
} Partition;

static inline bool Partition_ReadSectors(Partition *p, uint32_t lba, uint32_t count, void *buf) {
    if (!p || !p->dev || !p->dev->read_sectors) return false;
    // Route to underlying block device with base offset
    return p->dev->read_sectors(p->dev, p->lba_base + lba, count, buf) == 0;
}
