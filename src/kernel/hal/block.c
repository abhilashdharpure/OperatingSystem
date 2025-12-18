#include "block.h"
#include "hal/process.h"
#include "hal/ata.h"
#include "arch/i686/io.h"
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
    uint8_t status = i686_inb(ATA_STATUS);
    return !(status & ATA_BSY) && (status & ATA_DRQ);
}

void read_256_words_to(uint16_t *buf) {
    for (int i = 0; i < 256; i++) {
        buf[i] = i686_inw(ATA_DATA);
    }
}

// static ata_device_t ata0_master = {
//     .io_base = 0x1F0,
//     .device  = 0,  // master
// };
static ata_device_t ata0_master = { .device = 0, .io_base = 0x1F0, .ctrl_base = 0x3F6 };
// static block_device_t root_part = {
//     .read_sectors = ata_read_sectors,
//     .lba_base     = 2048,   // typical first partition start LBA; adjust to your disk
//     .sector_size  = 512,
//     .private_data = &ata0_master /* controller/port pointer */
// };

// void register_block_device(const char *name, block_device_t *dev)
// {
//     log_info("BLOCK", "register_block_device name =%s", name);

//     if (device_count < MAX_BLOCK_DEVICES) {
//         devices[device_count].name = name;
//         devices[device_count].dev = dev;
//         device_count++;
//     }
// }

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

    // register_mbr_partitions(&sda);
    // log_info("BLOCK", "Registered raw disk 'sda' lba_base=%u, sector_size = %u", sda.lba_base, sda.sector_size);


    //             // dev = &sda (raw disk)
    // uint32_t lba_start;
    // int rc = find_fat32_partition(&sda, &lba_start);

    // if (rc == 0) {
    //     // create or update partition device structure for sda1 with lba_base = lba_start
    //     block_device_t part = sda;                // copy base device fields
    //     part.lba_base = sda.lba_base + lba_start; // partition's absolute base
    //     // call your register method for partition devices, or use part directly:
    //     fat32_t *fs = fat32_init_device(&part);
    //     if (fs) {
    //         // success: store fs / vfs mount etc.
    //     }
    // } else {
    //     // no MBR or no FAT32 partition found - try superfloppy (filesystem directly at LBA 0)
    //     log_info("PART", "Trying raw device as FAT32 at LBA 0");
    //     block_device_t raw = sda;
    //     raw.lba_base = sda.lba_base + 0;
    //     fat32_t *fs = fat32_init_device(&raw);
    //     if (fs) {
    //         // success; mount
    //     } else {
    //         log_info("PART", "No FAT32 found on raw device");
    //     }
    // }


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

block_device_t *block_lookup_by_name(const char *name)
{

    // log_info("BLOCK", "block_lookup_by_name name = %s",name);

    for (int i = 0; i < MAX_BLOCK_DEVICES; i++)
    {
        // if (block_devices[i] && block_devices[i]->name)
        //     log_info("BLOCK", "block_lookup_by_name i = %d, name = %s", i, block_devices[i]->name);
        // else
        //     log_info("BLOCK", "block_lookup_by_name i = %d, empty", i);
            
        if (block_devices[i] && strcmp(block_devices[i]->name, name) == 0)
        {
            // log_info("BLOCK", "returning block_lookup_by_name name = %s",name);

            return block_devices[i];
        }
    }

    // for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
    //     block_device_t *dev = block_devices[i];
    //     if (dev && lba >= dev->lba_base) {
    //         return dev;
    //     }
    // }

    // log_info("BLOCK", "returning NULL from block_lookup_by_name  = %s",name);

    return NULL;
}

// block_device_t* block_lookup(uint32_t lba) {
//     for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
//         block_device_t *dev = block_devices[i];
//         if (dev && lba >= dev->lba_base) {
//             return dev;
//         }
//     }
//     return NULL;
// }

// block_device_t *block_lookup_by_lba(uint32_t lba) {
//     for (int i = 0; i < MAX_BLOCK_DEVICES; i++) {
//         if (block_devices[i] && lba >= block_devices[i]->lba_base &&
//             lba < block_devices[i]->lba_base + block_devices[i]->num_sectors) {
//             return block_devices[i];
//         }
//     }
//     return NULL;
// }


// block_device_t *block_lookup(const char *name)
// {
//     log_info("BLOCK", "block_lookup lba_base=%u", sda.lba_base);

//     // Temporary: return the single partition device
//     if (strcmp(name, "sda1") == 0)
//     {
//         return &sda;
//     }

//     log_info("BLOCK", "block_lookup returning NULL");

//     return NULL;
// }

// // elf.c
// pid_t exec_elf_mem(void *data, size_t size)
// {
//     log_debug("BLOCK", "exec_elf_mem Start, size = %u", size);
//     // TODO: implement ELF loader
//     return -1;
// }
