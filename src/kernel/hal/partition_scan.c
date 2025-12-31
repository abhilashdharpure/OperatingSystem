// partition_scan.c
#include "partition.h"
#include "block.h"
#include "debug.h"
#include "string.h"
#include "memory.h"

#pragma pack(push, 1)
typedef struct {
    uint8_t  status;
    uint8_t  chs_first[3];
    uint8_t  type;
    uint8_t  chs_last[3];
    uint32_t lba_first;
    uint32_t sectors;
} MbrPart;
#pragma pack(pop)

// Static block device for first FAT32 partition
static block_device_t sda1;

static int detect_first_fat32(block_device_t *disk, uint32_t *out_lba_base)
{
    if (!disk || !disk->read_sectors || !out_lba_base)
        return -1;

    uint8_t mbr[512];

    if (disk->read_sectors(disk, 0, 1, mbr) != 0) {
        log_error("PART", "Failed to read MBR sector from '%s'", disk->name);
        return -1;
    }

    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        log_error("PART", "Invalid MBR signature: 0x%02x%02x", mbr[510], mbr[511]);
        return -1;
    }

    MbrPart *parts = (MbrPart *)(mbr + 446);

    for (int i = 0; i < 4; i++) {
        log_debug("PART", "Partition %d: type=0x%02x lba_first=%u sectors=%u",
                  i, parts[i].type, parts[i].lba_first, parts[i].sectors);

        if (parts[i].type == 0x0B || parts[i].type == 0x0C) {
            *out_lba_base = parts[i].lba_first;
            log_info("PART", "Detected FAT32 partition at LBA %u (entry %d)",
                     *out_lba_base, i);
            return 0;
        }
    }

    log_error("PART", "No FAT32 partition found in MBR");
    return -1;
}

// Public: register first FAT32 partition as "sda1"
bool register_first_fat32_partition(void)
{
    block_device_t *disk = block_lookup_by_name("sda");
    if (!disk) {
        log_error("PART", "Disk 'sda' not found");
        return false;
    }

    uint32_t lba_base = 0;
    if (detect_first_fat32(disk, &lba_base) != 0) {
        log_error("PART", "No FAT32 partition found on 'sda'");
        return false;
    }

    if (lba_base == 0) {
        log_error("PART", "detect_first_fat32 returned lba_base == 0 (invalid)");
        return false;
    }

    memset(&sda1, 0, sizeof(sda1));
    sda1.read_sectors = disk->read_sectors;
    sda1.lba_base     = lba_base;            // partition base on disk
    sda1.sector_size  = disk->sector_size;
    sda1.private_data = disk->private_data;  // same ATA device

    if (register_block_device("sda1", &sda1) != 0) {
        log_error("PART", "Failed to register 'sda1'");
        return false;
    }

    log_info("PART", "Registered partition 'sda1' lba_base=%u", sda1.lba_base);
    return true;
}

// Optional helper for your own Partition type
Partition make_partition_from(const char *name)
{
    Partition p = {0};
    block_device_t *dev = block_lookup_by_name(name);
    if (dev) {
        p.dev         = dev;
        p.lba_base    = dev->lba_base;
        p.sector_size = dev->sector_size;
    }
    return p;
}
