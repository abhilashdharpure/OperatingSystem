#include "block.h"
#include "hal/process.h"
#include "hal/ata.h"
#include "arch/x86_64/io.h"
#include "debug.h"
#include "hal/ata.h"
#include "hal/fat32.h"


#define MAX_BLOCK_DEVICES 8
static struct {
    const char *name;
    block_device_t *dev;
} devices[MAX_BLOCK_DEVICES];
static int device_count = 0;

block_device_t *block_devices[MAX_BLOCK_DEVICES] = {0};


void issue_ata_read(uint32_t lba, uint32_t count) {
    // outb to ATA ports to set up the read command
}

int data_ready(void) {
    uint8_t status = inb(ATA_STATUS);
    return !(status & ATA_BSY) && (status & ATA_DRQ);
}

void read_256_words_to(uint16_t *buf) {
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_DATA);
    }
}

static ata_device_t ata0_master = { .device = 0, .io_base = 0x1F0, .ctrl_base = 0x3F6 };

int register_block_device(const char *name, block_device_t *dev)
{
    // log_info("BLOCK", "register_block_device name = %s", name);

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        if (!block_devices[i]) {
            block_devices[i] = dev;
            strncpy(dev->name, name, sizeof(dev->name));
            dev->name[sizeof(dev->name) - 1] = '\0';
            return 0;
        }
    }

    log_error("BLOCK", "register_block_device failed: registry full");
    return -1;
}



static block_device_t sda = {
    .read_sectors = ata_read_sectors,
    .lba_base     = 0, // raw disk
    // .lba_base     = 2048, // raw disk
    .sector_size  = 512,
    .private_data = &ata0_master
};

void block_init(void)
{
    // Detect controller, initialize ports
    // For now, just register ata0_master
    register_block_device("sda", &sda);
    log_info("BLOCK", "Registered raw disk 'sda' lba_base=%u, sector_size = %u", sda.lba_base, sda.sector_size);
}



void block_register(block_device_t *dev)
{
    for (int i = 0; i < MAX_BLOCK_DEVICES; i++)
    {
        if (!block_devices[i])
        {
            block_devices[i] = dev;
            return;
        }
    }
}

block_device_t *block_find_raw_disk(void)
{
    for (int i = 0; i < MAX_BLOCK_DEVICES; i++)
    {
        if (!block_devices[i]) continue;

        if (block_devices[i]->lba_base == 0)
            return block_devices[i];
    }
    return NULL;
}


block_device_t *block_lookup_by_name(const char *name)
{

    log_info("BLOCK", "block_lookup_by_name name = %s",name);

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++)
    {
        log_info("BLOCK", "inside for loop i = %d",i);

        if (block_devices[i] && strcmp(block_devices[i]->name, name) == 0)
        {
            log_info("BLOCK", "returning block_lookup_by_name name = %s",name);

            return block_devices[i];
        }
    }

    return NULL;
}
