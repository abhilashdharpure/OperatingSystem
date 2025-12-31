// block.c
#include "block.h"
#include "ata.h"
#include "string.h"
#include "debug.h"

#define MAX_BLOCK_DEVICES 8

block_device_t *block_devices[MAX_BLOCK_DEVICES] = {0};

int register_block_device(const char *name, block_device_t *dev)
{
    if (!name || !dev) return -1;

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        if (!block_devices[i]) {
            block_devices[i] = dev;
            strncpy(dev->name, name, sizeof(dev->name));
            dev->name[sizeof(dev->name) - 1] = '\0';

            log_info("BLOCK", "Registered block device '%s' lba_base=%u sector_size=%u",
                     dev->name, dev->lba_base, dev->sector_size);
            return 0;
        }
    }

    log_error("BLOCK", "register_block_device failed: registry full");
    return -1;
}

block_device_t *block_lookup_by_name(const char *name)
{
    if (!name) return NULL;

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        if (block_devices[i] && strcmp(block_devices[i]->name, name) == 0)
            return block_devices[i];
    }
    return NULL;
}

block_device_t *block_find_raw_disk(void)
{
    for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
        if (block_devices[i] && block_devices[i]->lba_base == 0)
            return block_devices[i];
    }
    return NULL;
}

// Primary raw disk "sda" wrapping ATA0 master
static block_device_t sda;

void block_init(void)
{
    // Initialize ATA hardware
    ata_init();

    if (!ata_primary_present())
    {
        log_error("BLOCK", "ATA primary master not present; no 'sda' registered");
        return;
    }

    // Wrap ata0_master as raw disk sda
    memset(&sda, 0, sizeof(sda));
    sda.read_sectors = ata_read_sectors;
    sda.lba_base     = 0;       // raw disk
    sda.sector_size  = 512;
    sda.private_data = ata_get_primary_master();

    register_block_device("sda", &sda);
}
