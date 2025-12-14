// partition_scan.c
#include "partition.h"
#include "block.h"
#include "ata.h"
#include "string.h"
#include "debug.h"

typedef struct __attribute__((packed)) {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba_first;
    uint32_t sectors;
} MbrPart;

// Scan MBR for the first FAT32 partition (0x0B or 0x0C)
int detect_first_fat32(block_device_t *disk, uint32_t *out_lba_base) {
    if (!disk || !disk->read_sectors || !out_lba_base) return -1;

    uint8_t mbr[512];
    if (disk->read_sectors(disk, 0, 1, mbr) != 0) {
        log_error("PART", "Failed to read MBR sector");
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
            log_debug("PART", "Detected FAT32 partition at LBA %u", *out_lba_base);
            return 0;
        }
    }

    log_debug("PART", "MBR[510..511]=0x%02x%02x", mbr[510], mbr[511]);
    for (int i=0;i<4;i++)
        log_debug("PART", "Partition %d: type=0x%02x lba_first=%u", i, parts[i].type, parts[i].lba_first);



    log_error("PART", "No FAT32 partition found in MBR");
    return -1;
}

// ATA devices
static ata_device_t ata0_master = { .device = 0, .io_base = 0x1F0, .ctrl_base = 0x3F6 };
static ata_device_t ata0_slave  = { .device = 1, .io_base = 0x1F0, .ctrl_base = 0x3F6 };

// Raw disk device for reading MBR
static block_device_t sda = {
    .read_sectors = ata_read_sectors,
    .lba_base     = 0,
    .sector_size  = 512,
    .private_data = &ata0_master
};

// Partition device for FAT32
static block_device_t sda1 = {
    .read_sectors = ata_read_sectors,
    .lba_base     = 0, // will be updated after MBR scan
    .sector_size  = 512,
    .private_data = &ata0_master
};

// Register first FAT32 partition
bool register_first_fat32_partition(void)
{
    // log_info("PART", "register_first_fat32_partition start");

    block_device_t *disk = block_lookup_by_name("sda"); // the whole-disk device
    if (!disk)
    {
        log_error("PART", "Disk 'sda' not found");
        return false;
    }

    uint32_t lba_base = 0;
    if (detect_first_fat32(disk, &lba_base) != 0)
    {
        log_error("PART", "No FAT32 partition found on raw disk");
        return false;
    }

    if (lba_base == 0)
    {
        log_error("PART", "detect_first_fat32 returned lba_base == 0 (invalid)");
        return false;
    }

    sda1.lba_base = lba_base;
    sda1.private_data = disk->private_data; // keep same ata device pointer
    register_block_device("sda1", &sda1);

    // log_info("PART", "Registered sda1 -> lba_base=%u (0x%x) sector_size=%u",
    //          sda1.lba_base, sda1.lba_base, sda1.sector_size);
    return true;
}


// Utility: create a Partition struct from a registered device
Partition make_partition_from(const char *name)
{
    // log_info("PART", "make_partition_from name = %s", name);

    Partition p = {0};
    block_device_t *dev = block_lookup_by_name(name);
    if (dev) {
        p.dev = dev;
        p.lba_base = dev->lba_base;
        p.sector_size = dev->sector_size;
    }

    // log_info("PART", "make_partition_from p.lba_base = %u", p.lba_base);
    // log_info("PART", "make_partition_from p.sector_size = %u", p.sector_size);

    return p;
}
