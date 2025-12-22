#include "ata.h"
#include "debug.h"
#include "stdbool.h"


static void io_wait_400ns(void) {
    for (volatile int i = 0; i < 1000; i++);
}


// 'dev' is 0 for master, 1 for slave
// 'lba' is the 28-bit LBA sector address
static inline uint8_t ata_device_reg(uint8_t dev, uint32_t lba) {
    return 0xE0 |          // LBA mode + default bits
           ((dev & 1) << 4) |   // master/slave select
           ((lba >> 24) & 0x0F); // top 4 bits of LBA28
}

static ata_device_t ata0_master = { .device = 0, .io_base = 0x1F0, .ctrl_base = 0x3F6 };
static ata_device_t ata0_slave  = { .device = 1, .io_base = 0x1F0, .ctrl_base = 0x3F6 };

// Put these into ata.c (replace current functions)
int ata_probe_identify(void) {
    log_info("ATA", "IDENTIFY probe start");
    // select master
    // outb(ATA_DEVICE, 0xA0); // 0xA0 = master for IDENTIFY legacy

    ata_device_t *ata = &ata0_master; // or from private_data
    outb(ata->io_base + ATA_DEVICE, 0xA0 | (ata->device << 4));
    io_wait_400ns();

    // zero sector count and LBA registers per spec
    outb(ATA_SECCNT, 0);
    outb(ATA_LBA_LO, 0);
    outb(ATA_LBA_MID, 0);
    outb(ATA_LBA_HI, 0);

    // issue IDENTIFY DEVICE (0xEC)
    outb(ATA_COMMAND, 0xEC);
    uint16_t io_base = ATA_IO_BASE;
    // wait for response up to a short deadline
    for (int i = 0; i < 10000; ++i) {
        uint8_t st = inb(io_base + ATA_STATUS);
        if (st == 0) {
            // no device responding
            if ((i & 255) == 0) log_info("ATA", "identify loop still 0 (i=%d)", i);
            io_wait_400ns();
            continue;
        }
        if (st & ST_ERR) { log_error("ATA", "IDENTIFY: ERR set (st=0x%02x)", st); return -2; }
        if (!(st & ST_BSY) && (st & ST_DRQ)) {
            log_info("ATA", "IDENTIFY: device answered (st=0x%02x)", st);
            return 0;
        }
        io_wait_400ns();
    }

    log_info("ATA", "IDENTIFY: no response");
    return -1;
}


int ata_wait_not_busy(ata_device_t *ata, uint32_t timeout_ms) {
    // simplified polling loop
    while (inb(ata->io_base + ATA_STATUS) & ST_BSY);
    return 0;
}

// Wait for ATA device DRQ (data ready)
int ata_wait_drq(ata_device_t *ata, uint32_t timeout_ms)
{
    uint64_t start = pit_get_ticks();
    log_info("ATA", "ata_wait_drq start = %llu", start);

    while ((pit_get_ticks() - start) < timeout_ms) {
        uint8_t st = inb(ata->io_base + ATA_STATUS);
        
        log_info("ATA", "ata_wait_drq status = 0x%02x", st);

        if (!(st & ST_BSY) && (st & ST_DRQ)) {
            return 0; // data ready
        }
        io_wait_400ns();
    }

    log_error("ATA", "ata_wait_drq TIMEOUT after %u ms", timeout_ms);
    return -1;
}


static void ata_select_drive_master(void) {
    log_debug("ATA", "Start ata_select_drive_master");
    outb(ATA_DEVICE, 0xE0); // master, high 4 bits of LBA later
    io_wait_400ns();
    log_debug("ATA", "End ata_select_drive_master");
}

static int ata_identify(ata_device_t *ata) {
    log_info("ATA", "IDENTIFY probe device=%u", ata->device);

    outb(ata->io_base + ATA_DEVICE, 0xA0 | (ata->device << 4));
    io_wait_400ns(); 

    outb(ata->io_base + ATA_SECCNT, 0);
    outb(ata->io_base + ATA_LBA_LO, 0);
    outb(ata->io_base + ATA_LBA_MID, 0);
    outb(ata->io_base + ATA_LBA_HI, 0);
    outb(ata->io_base + ATA_COMMAND, 0xEC); // IDENTIFY

    io_wait_400ns();

    if (ata_wait_not_busy(ata, 5000) != 0) {
        log_error("ATA", "Device %u did not respond", ata->device);
        return -1;
    }

    for (int i = 0; i < 10000; ++i) {
        uint8_t st = inb(ata->io_base + ATA_STATUS);
        if (st == 0)
        { 
            io_wait_400ns();
            continue; 
        }
        if (st & ST_ERR) { 
            log_error("ATA", "return if (st & ST_ERR)");
            return -2;
        }
        if (!(st & ST_BSY) && (st & ST_DRQ))
        { 
            return 0;
        }
        io_wait_400ns();
    }

    uint8_t status = inb(ata->io_base + ATA_STATUS);
    if (status == 0) {
        log_error("ATA", "No device on channel %u", ata->device);
        return -1;
    }

    log_info("ATA", "Device %u ready (STATUS=0x%02x)", ata->device, status);
    return 0;
}


static void issue_ata_read28(uint32_t lba, uint8_t count) {
    outb(ATA_DEV_CTRL, 0x00);       // enable interrupts
    outb(ATA_SECCNT, count);
    outb(ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_DEVICE, 0xE0 | ((lba >> 24) & 0x0F)); // master + LBA high nibble
    outb(ATA_COMMAND, 0x20);       // READ SECTORS (PIO)
}

static void read_512_bytes(void *buf){
    uint16_t *w = (uint16_t*)buf;
    for (int i = 0; i < 256; ++i) w[i] = inw(ATA_DATA);
}

// int ata_read_sectors(block_device_t *dev, uint32_t lba, uint32_t count, void *buf)
// {
//     ata_device_t *ata = (ata_device_t *)dev->private_data;
//     uint8_t device = ata->device;
//     uint16_t io_base = ata->io_base;
//     uint8_t *buffer = buf;

//     if (ata_wait_not_busy(ata, 5000) < 0) return -1;

//     outb(io_base + ATA_SECCNT, (uint8_t)count);
//     io_wait_400ns();
//     outb(io_base + ATA_LBA_LO,  (uint8_t)(lba & 0xFF));
//     io_wait_400ns();
//     outb(io_base + ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
//     io_wait_400ns();
//     outb(io_base + ATA_LBA_HI,  (uint8_t)((lba >> 16) & 0xFF));
//     io_wait_400ns();
//     outb(io_base + ATA_DEVICE,  0xE0 | ((device & 1) << 4) | ((lba >> 24) & 0x0F));
//     io_wait_400ns();

//     outb(io_base + ATA_COMMAND, ATA_CMD_READ_SECTORS);
//     io_wait_400ns();

//     for (uint32_t i = 0; i < count; i++) {
//         if (ata_wait_drq(ata, 5000) < 0) return -2;

//         for (int j = 0; j < 256; j++) {
//             uint16_t word = inw(io_base + ATA_DATA);
//             buffer[j*2]   = word & 0xFF;
//             buffer[j*2+1] = (word >> 8) & 0xFF;
//         }

//         buffer += 512; // move to next sector
//     }

//     // debug: dump first 16 bytes (decimal) like you already do
//     for (int i=0;i<16;i++) {
//         log_info("FAT32", "vbr ((uint8_t*)buf)[%d]=%d", i, ((uint8_t*)buf)[i]);
//     }

//     // *** DO NOT validate VBR signature here. ***
//     // The block device driver must only read sectors; parsing/validation is the
//     // caller's job (partition parser / filesystem). Return success.
//     return 0;
// }


int ata_read_sectors(block_device_t *dev, uint32_t lba, uint32_t count, void *buf) {
    ata_device_t *ata = (ata_device_t*)dev->private_data;

    // select master
    outb(ata->io_base + ATA_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ata->io_base + ATA_SECCNT, count);
    outb(ata->io_base + ATA_LBA_LO, lba & 0xFF);
    outb(ata->io_base + ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ata->io_base + ATA_LBA_HI, (lba >> 16) & 0xFF);
    outb(ata->io_base + ATA_COMMAND, ATA_CMD_READ_SECTORS);

    for (uint32_t s = 0; s < count; s++) {
        while (!(inb(ata->io_base + ATA_STATUS) & ST_DRQ));
        for (int i = 0; i < 256; i++) {
            ((uint16_t*)buf)[i + s*256] = inw(ata->io_base + ATA_DATA);
        }
    }

    return 0;
}

static ata_device_t ata_master = {0, ATA_IO_BASE, ATA_CTRL_BASE};
void ata_init() {
    static block_device_t ata_dev = {0};
    ata_dev.read_sectors = ata_read_sectors;
    ata_dev.lba_base = 2048;
    ata_dev.sector_size = 512;
    ata_dev.private_data = &ata_master;
    // ata_dev.name = "sda";

    block_register(&ata_dev);
}



// extern ata_device ata_devices[4];

// static inline void io_wait_400ns(void) {
//     (void)inb(ATA_ALT_STATUS);
//     (void)inb(ATA_ALT_STATUS);
//     (void)inb(ATA_ALT_STATUS);
//     (void)inb(ATA_ALT_STATUS);
// }

// static inline void io_wait_400ns(void) {
//     inb(0x80); inb(0x80); inb(0x80); inb(0x80);
// }
// Wait for ATA device to clear BSY
// int ata_wait_not_busy(ata_device_t *ata, uint32_t timeout_ms)
// {
//     uint64_t start = pit_get_ticks();
//     log_info("ATA", "ata_wait_not_busy start = %llu", start);

//     while ((pit_get_ticks() - start) < timeout_ms) {
//         uint8_t st = inb(ata->io_base + ATA_STATUS);
//         log_info("ATA", "ata_wait_not_busy status = 0x%02x", st);

//         if (!(st & ST_BSY)) {
//             return 0; // device not busy
//         }
//         io_wait_400ns();
//     }

//     log_error("ATA", "ata_wait_not_busy TIMEOUT after %u ms", timeout_ms);
//     return -1;
// }

// int ata_wait_not_busy(ata_device_t *ata, uint32_t timeout_ms)
// {
//     log_error("ATA", "Inside ata_wait_not_busy");

//     uint64_t start = pit_get_ticks();
//     log_error("ATA", "Inside ata_wait_not_busy, start = ", start);
//     while ((pit_get_ticks() - start) < timeout_ms)
//     {
//         uint8_t st = inb(ata->io_base + ATA_STATUS);
//         log_error("ATA", "Inside ata_wait_not_busy, st = ", st);

//         if (!(st & ST_BSY))
//         {
//             return 0;
//         }
//         io_wait_400ns();
//     }
//     log_error("ATA", "wait_not_busy timeout");
//     return -1;
// }

// int ata_wait_drq(ata_device_t *ata, uint32_t timeout_ms) {
//     uint64_t start = pit_get_ticks();
//     while ((pit_get_ticks() - start) < timeout_ms) {
//         uint8_t st = inb(ata->io_base + ATA_STATUS);
//         if (!(st & ST_BSY) && (st & ST_DRQ)) return 0;
//         io_wait_400ns();
//     }
//     log_error("ATA", "wait_drq timeout");
//     return -1;
// }
// void ata_init()
// {
//     // log_debug("ATA", "ata_init");
//     log_info("ATA", "Initializing ATA devices");

//     if (ata_identify(&ata0_master) == 0)
//     {
//         log_info("ATA", "Master device detected");
//          //block_register(&ata0_master);
//     } else {
//         log_error("ATA", "Master device missing");
//     }

//     if (ata_identify(&ata0_slave) == 0)
//     {
//         log_info("ATA", "Slave device detected");
//         //block_register(&ata0_slave);
//     } else {
//         log_error("ATA", "Slave device missing");
//     }
// }
