#include <stdint.h>
#include "hal/block.h"
#include "paging.h"

static uint8_t *rd_base;
static uint64_t rd_size;

static int rd_read(block_device_t *b, uint32_t lba, uint32_t count, void *buf)
{
    if (((uint64_t)lba + count) * 512 > rd_size) return -1;
    memcpy(buf, rd_base + (uint64_t)lba * 512, (size_t)count * 512);
    return 0;
}
static int rd_write(block_device_t *b, uint32_t lba, uint32_t count, const void *buf)
{
    if (((uint64_t)lba + count) * 512 > rd_size) return -1;
    memcpy(rd_base + (uint64_t)lba * 512, buf, (size_t)count * 512);
    return 0;
}

static block_device_t rd_dev;
int ramdisk_register(uint64_t phys, uint64_t size)
{
    rd_base = (uint8_t *)(DIRECT_MAP_BASE + phys);
    rd_size = size;
    memset(&rd_dev, 0, sizeof rd_dev);
    rd_dev.read_sectors = rd_read;
    rd_dev.write_sectors = rd_write;
    rd_dev.lba_base = 0;
    rd_dev.sector_size = 512;
    return register_block_device("sda", &rd_dev);
}