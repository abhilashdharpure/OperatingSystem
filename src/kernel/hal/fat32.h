// fat32.h
#pragma once
#include <stdint.h>
#include "block.h"
#include "file.h"

typedef struct fat32 {
    block_device_t *bdev;

    // BPB fields
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint32_t sectors_per_fat;
    uint32_t root_cluster; // typically 2

    // computed
    uint32_t fat_start_lba;     // start of FAT region
    uint32_t data_start_lba;    // start of data region
} fat32_t;

typedef struct fat32_node {
    fat32_t *fs;
    uint32_t first_cluster;
    uint32_t size;      // for files; 0 for dirs
    uint8_t  is_dir;    // 1=dir, 0=file
    // cursor for readdir
    uint32_t cur_cluster;
    uint32_t cur_offset; // byte offset within current cluster
    // short name cache for debugging
    char name[NAME_MAX];
} fat32_node_t;


static int fat32_open(struct file *f, int flags);
static int fat32_close(struct file *f);
static int fat32_readdir(struct file *f, dirent_t *e);
static int fat32_read(struct file *f, void *buf, size_t size);
static int fat_stat(struct file *f, struct kstat *st);

struct file_operations *get_fat32_fops(void);