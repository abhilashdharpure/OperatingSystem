// ata.c
#include "ata.h"
#include "block.h"
#include "arch/x86_64/io.h"
#include "debug.h"

static int ata0_present = 0;

static ata_device_t ata0_master = {
    .device    = 0,          // master
    .io_base   = ATA_IO_BASE,
    .ctrl_base = ATA_CTRL_BASE,
};

// simple 400ns wait
static inline void ata_io_wait_400ns(uint16_t io_base) {
    (void)inb(io_base + ATA_REG_STATUS);
    (void)inb(io_base + ATA_REG_STATUS);
    (void)inb(io_base + ATA_REG_STATUS);
    (void)inb(io_base + ATA_REG_STATUS);
}

static int ata_wait_bsy_clear(uint16_t io_base) {
    for (volatile int i = 0; i < 5000000; i++) {
        uint8_t st = inb(io_base + ATA_REG_STATUS);
        if (!(st & ATA_SR_BSY))
            return 0;
        ata_io_wait_400ns(io_base);
    }
    return -1;
}

static int ata_wait_drq(uint16_t io_base) {
    for (volatile int i = 0; i < 5000000; i++) {
        uint8_t st = inb(io_base + ATA_REG_STATUS);
        if (st & ATA_SR_ERR) return -2;
        if (!(st & ATA_SR_BSY) && (st & ATA_SR_DRQ))
            return 0;
        ata_io_wait_400ns(io_base);
    }
    return -1;
}

// Public read: dev->private_data must point to ata_device_t
int ata_read_sectors(block_device_t *dev, uint32_t lba, uint32_t count, void *buf)
{
    if (!dev || !dev->private_data || !buf || count == 0)
        return -1;

    ata_device_t *ata = (ata_device_t*)dev->private_data;
    uint16_t io = ata->io_base;

    while (count > 0) {
        uint8_t sectors = (count > 255) ? 255 : (uint8_t)count;

        if (ata_wait_bsy_clear(io) != 0) {
            log_error("ATA", "wait_bsy_clear timeout before read");
            return -1;
        }

        // device select + high LBA bits
        outb(io + ATA_REG_DEVICE,
             0xE0 | (ata->device << 4) | ((lba >> 24) & 0x0F));
        ata_io_wait_400ns(io);

        // sector count + LBA0..23
        outb(io + ATA_REG_SECCNT, sectors);
        outb(io + ATA_REG_LBA_LO,  (uint8_t)(lba & 0xFF));
        outb(io + ATA_REG_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
        outb(io + ATA_REG_LBA_HI,  (uint8_t)((lba >> 16) & 0xFF));

        // READ SECTORS
        outb(io + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

        uint16_t *pbuf = (uint16_t*)buf;

        for (uint8_t s = 0; s < sectors; s++) {
            if (ata_wait_drq(io) != 0) {
                log_error("ATA", "wait_drq failed");
                return -2;
            }

            // 512 bytes = 256 words
            for (int i = 0; i < 256; i++) {
                *pbuf++ = inw(io + ATA_REG_DATA);
            }
        }

        lba   += sectors;
        buf    = (void*)pbuf;
        count -= sectors;
    }

    return 0;
}

void ata_init(void)
{
    ata_device_t *ata = &ata0_master;
    uint16_t io   = ata->io_base;
    uint16_t ctrl = ata->ctrl_base;

    log_info("ATA", "Initializing ATA primary master (PIO LBA28)...");

    // Software reset
    outb(ctrl, 0x04);
    for (volatile int i = 0; i < 10000; i++);
    outb(ctrl, 0x00);
    for (volatile int i = 0; i < 10000; i++);

    // Select master
    outb(io + ATA_REG_DEVICE, 0xA0 | (ata->device << 4));
    for (volatile int i = 0; i < 10000; i++);

    // Clear BSY
    uint8_t st;
    for (int i = 0; i < 100000; i++) {
        st = inb(io + ATA_REG_STATUS);
        if (!(st & ATA_SR_BSY))
            break;
    }

    // Issue IDENTIFY
    outb(io + ATA_REG_SECCNT, 0);
    outb(io + ATA_REG_LBA_LO, 0);
    outb(io + ATA_REG_LBA_MID, 0);
    outb(io + ATA_REG_LBA_HI, 0);
    outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    // If status becomes 0 here and stays 0 => no device
    st = inb(io + ATA_REG_STATUS);
    if (st == 0) {
        log_error("ATA", "No device on primary master (status stayed 0)");
        return;
    }

    // Poll until BSY==0 and either DRQ or ERR is set, or timeout
    for (int i = 0; i < 100000; i++) {
        st = inb(io + ATA_REG_STATUS);
        if (!(st & ATA_SR_BSY) && (st & (ATA_SR_DRQ | ATA_SR_ERR)))
            break;
    }

    if (st & ATA_SR_ERR) {
        log_error("ATA", "IDENTIFY: ERR set (status=0x%02x)", st);
        return;
    }
    if (!(st & ATA_SR_DRQ)) {
        log_error("ATA", "IDENTIFY: no DRQ (not an ATA disk?) status=0x%02x", st);
        return;
    }

    uint16_t id[256];
    for (int i = 0; i < 256; i++)
        id[i] = inw(io + ATA_REG_DATA);

    uint32_t sectors = id[60] | ((uint32_t)id[61] << 16);
    log_info("ATA", "Primary master ready: %u sectors (~%u MiB)",
             sectors, sectors / 2048);


    ata0_present = 1;
}

// Provide a way for block.c to get the ATA device
ata_device_t *ata_get_primary_master(void)
{
    return &ata0_master;
}

int ata_primary_present(void)
{
    return ata0_present;
}