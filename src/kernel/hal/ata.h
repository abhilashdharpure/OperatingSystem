#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "block.h"

// Primary ATA channel I/O ports
#define ATA_IO_BASE      0x1F0
#define ATA_CTRL_BASE    0x3F6

// ATA Registers (offsets)
#define ATA_REG_DATA     0   // Data (R/W)
#define ATA_REG_ERROR    1   // Error (R)
#define ATA_REG_FEATURES 1   // Features (W)
#define ATA_REG_SECCNT   2
#define ATA_REG_LBA_LO   3
#define ATA_REG_LBA_MID  4
#define ATA_REG_LBA_HI   5
#define ATA_REG_DEVICE   6
#define ATA_REG_STATUS   7   // Status (R)
#define ATA_REG_COMMAND  7   // Command (W)

// Status bits
#define ATA_SR_ERR   0x01
#define ATA_SR_DRQ   0x08
#define ATA_SR_DF    0x20
#define ATA_SR_DRDY  0x40
#define ATA_SR_BSY   0x80

// Commands
#define ATA_CMD_IDENTIFY       0xEC
#define ATA_CMD_READ_SECTORS   0x20

// Assumes in ata.h:
//   #define ATA_IO_BASE   0x1F0
//   #define ATA_CTRL_BASE 0x3F6
//   #define ATA_REG_DATA     0x00
//   #define ATA_REG_ERROR    0x01
//   #define ATA_REG_SECCNT   0x02
//   #define ATA_REG_LBA_LO   0x03
//   #define ATA_REG_LBA_MID  0x04
//   #define ATA_REG_LBA_HI   0x05
//   #define ATA_REG_DEVICE   0x06
//   #define ATA_REG_COMMAND  0x07
//   #define ATA_REG_STATUS   0x07
//   #define ATA_SR_BSY       0x80
//   #define ATA_SR_DRQ       0x08
//   #define ATA_SR_ERR       0x01
//   #define ATA_CMD_IDENTIFY     0xEC
//   #define ATA_CMD_READ_SECTORS 0x20
//
// typedef struct {
//     uint8_t  device;    // 0=master,1=slave
//     uint16_t io_base;
//     uint16_t ctrl_base;
// } ata_device_t;
// Public API
void ata_init(void);

// Block-layer wrapper
int ata_read_sectors(block_device_t *bdev, uint32_t lba, uint32_t count, void *buf);