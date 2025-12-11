#pragma once

#include "arch/i686/io.h"
#include "hal/block.h"


#define ATA_IO_BASE     0x1F0   // primary channel
#define ATA_CTRL_BASE   0x3F6   // control/status


#define ATA_DATA        0x00
#define ATA_ERROR       0x01
#define ATA_SECCNT      0x02
#define ATA_LBA_LO      0x03
#define ATA_LBA_MID     0x04
#define ATA_LBA_HI      0x05
#define ATA_DEVICE      0x06
#define ATA_STATUS      0x07
#define ATA_COMMAND     0x07

#define ATA_CMD_READ_SECTORS 0x20

#define ATA_ALT_STATUS  (ATA_CTRL_BASE + 0)
#define ATA_DEV_CTRL    (ATA_CTRL_BASE + 0)

#define ATA_BSY 0x80
#define ATA_DRQ 0x08
#define ATA_ERR 0x01
#define ATA_DF  0x20

// Status bits
#define ST_ERR 0x01
#define ST_DRQ 0x08
#define ST_DF  0x20
#define ST_BSY 0x80

static inline void io_wait_400ns(void);

int ata_wait_not_busy(ata_device_t *ata, uint32_t timeout_ms);
int ata_wait_drq(ata_device_t *ata, uint32_t timeout_ms);

static void ata_select_drive_master(void);

static void issue_ata_read28(uint32_t lba, uint8_t count);

static void read_512_bytes(void *buf);

int ata_read_sectors(block_device_t *dev, uint32_t lba, uint32_t count, void *buf);

void ata_init();