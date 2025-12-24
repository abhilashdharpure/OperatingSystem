#include "ata.h"
#include "debug.h"
#include "stdbool.h"

#define ATA_CMD_IDENTIFY 0xEC

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
    outb(ata->io_base + ATA_SECCNT, 0);
    outb(ata->io_base + ATA_LBA_LO, 0);
    outb(ata->io_base + ATA_LBA_MID, 0);
    outb(ata->io_base + ATA_LBA_HI, 0);
    // issue IDENTIFY DEVICE (0xEC)
    outb(ata->io_base + ATA_COMMAND, ATA_CMD_IDENTIFY);
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

int ata_wait_not_busy(uint16_t io)
{
    uint8_t st;
    for (int i = 0; i < 100000; i++) {
        st = inb(io + ATA_STATUS);
        if (!(st & ATA_BSY))
            return 0;
    }
    return -1;
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


static int ata_identify(ata_device_t *ata)
{
    log_debug("ATA", "ata_identify 1");

    // Select drive (legacy)
    outb(ata->io_base + ATA_DEVICE, 0xA0 | (ata->device << 4));
    log_debug("ATA", "ata_identify 2");

    ata_400ns_delay(ata->io_base);
    log_debug("ATA", "ata_identify 3");

    outb(ata->io_base + ATA_SECCNT, 0);
    outb(ata->io_base + ATA_LBA_LO, 0);
    outb(ata->io_base + ATA_LBA_MID, 0);
    outb(ata->io_base + ATA_LBA_HI, 0);
    log_debug("ATA", "ata_identify 4");

    outb(ata->io_base + ATA_COMMAND, ATA_CMD_IDENTIFY);
    log_debug("ATA", "ata_identify 5");

    uint8_t st;
    // for (;;) {
    //     st = inb(ata->io_base + ATA_STATUS);
    //     if (st & ATA_ERR) return -1;
    //     if (!(st & ATA_BSY) && (st & ATA_DRQ)) break;
    // }
    uint64_t start = pit_get_ticks();
    log_debug("ATA", "ata_identify start = %u", start);

    while ((pit_get_ticks() - start) < 500)
    { // 500 ms timeout
        // log_debug("ATA", "while loop");

        st = inb(ata->io_base + ATA_STATUS);
        // log_debug("ATA", "ata_identify st = %u", st);
        // log_info("ATA", "status before IDENTIFY = 0x%02x", st);

        if (st & ATA_ERR) return -1;
        if (!(st & ATA_BSY) && (st & ATA_DRQ)) break;
        io_wait_400ns();
    }
    if ((st & ATA_DRQ) == 0) {
        log_error("ATA", "IDENTIFY timed out, status=0x%02x", st);
        return -1;
    }

    log_debug("ATA", "ata_identify 6");

    for (int i = 0; i < 256; i++)
        inw(ata->io_base + ATA_DATA);


    log_info("ATA", "IDENTIFY successful");
    return 0;
}


void ata_400ns_delay(uint16_t io)
{
    inb(io + ATA_STATUS);
    inb(io + ATA_STATUS);
    inb(io + ATA_STATUS);
    inb(io + ATA_STATUS);
}

int ata_read_sectors(block_device_t *dev, uint32_t lba, uint32_t count, void *buf)
{
    log_error("ATA", "ata_read_sectors: lba=%u count=%u", lba, count);
    ata_device_t *ata = (ata_device_t*)dev->private_data;

    // uint8_t st = inb(ATA_STATUS);
    uint8_t st = inb(ata->io_base + ATA_STATUS);

    log_error("ATA", "STATUS initial = 0x%02x", st);


    // 1. Select drive
    outb(ata->io_base + ATA_DEVICE,
     0xE0 | (ata->device << 4) | ((lba >> 24) & 0x0F));
    ata_400ns_delay(ata->io_base);

    // Wait not busy
    if (ata_wait_not_busy(ata->io_base) != 0)
        return -1;

    // 2. Set parameters
    outb(ata->io_base + ATA_SECCNT, count);
    outb(ata->io_base + ATA_LBA_LO,  lba & 0xFF);
    outb(ata->io_base + ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ata->io_base + ATA_LBA_HI,  (lba >> 16) & 0xFF);

    // 3. Send command
    outb(ata->io_base + ATA_COMMAND, ATA_CMD_READ_SECTORS);
    ata_400ns_delay(ata->io_base);

    // 4. Wait for BSY clear
    while (inb(ata->io_base + ATA_STATUS) & ATA_BSY);

    // 5. Wait for DRQ or ERR
    uint8_t status;
    int spin = 0;

    while (1) {
        status = inb(ata->io_base + ATA_STATUS);

        if (status & ATA_ERR) {
            log_error("ATA", "ERR after READ, status=0x%02x", status);
            return -1;
        }

        if (status & ATA_DF) {
            log_error("ATA", "DF after READ, status=0x%02x", status);
            return -1;
        }

        if (status & ATA_DRQ)
            break;

        if (++spin > 1000000) {
            log_error("ATA", "Timeout waiting DRQ, status=0x%02x", status);
            return -1;
        }
    }

    // 6. Read data
    for (int i = 0; i < 256; i++)
    {
        ((uint16_t*)buf)[i] = inw(ata->io_base + ATA_DATA);
    }
    return 0;
}

static ata_device_t ata_master = {
    .device    = 0,
    .io_base   = ATA_IO_BASE,
    .ctrl_base = ATA_CTRL_BASE
};

void ata_init(void)
{
    log_info("ATA", "ata_init 1");

    static block_device_t ata_dev = {0};
    log_info("ATA", "ata_init 2");

    // Disable IRQs, clear reset
    outb(ata_master.ctrl_base, 0x02);
    log_info("ATA", "ata_init 3");

    ata_400ns_delay(ata_master.io_base);
    log_info("ATA", "ata_init 4");

    if (ata_identify(&ata_master) != 0) {
        log_error("ATA", "IDENTIFY failed, disk disabled");
        return;   // <-- do NOT register
    }
    log_info("ATA", "ata_init 5");


    ata_dev.read_sectors = ata_read_sectors;
    ata_dev.lba_base     = 0;        // MUST be 0
    ata_dev.sector_size  = 512;
    ata_dev.private_data = &ata_master;
    log_info("ATA", "ata_init 6");

    register_block_device("sda", &ata_dev);

    log_info("ATA", "ATA master initialized");
}

