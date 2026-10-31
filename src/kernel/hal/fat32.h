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

    uint32_t fat_cache_lba;       // fs-relative; 0xFFFFFFFF = empty
    uint8_t  fat_cache[512];

    uint32_t total_clusters;
    uint32_t next_free;            // allocation hint
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

    uint32_t seek_cluster;        // cluster at index seek_index (0 = none)
    uint32_t seek_index;

    uint32_t dirent_lba;           // FS-relative LBA of the sector holding the 8.3 entry (0 = root)
    uint16_t dirent_off;
    uint32_t parent_cluster;
} fat32_node_t;

// exported
int fat32_create_file(fat32_t *fs, const char *path, fat32_node_t *out);
int fat32_mkdir(fat32_t *fs, const char *path);
int fat32_unlink(fat32_t *fs, const char *path);               // files and empty folders
int fat32_rename(fat32_t *fs, const char *from, const char *to);


static int fat32_open(struct file *f, int flags);
static int fat32_close(struct file *f);
static int fat32_readdir(struct file *f, dirent_t *e);
static int fat32_read(struct file *f, void *buf, size_t size);
static int fat_stat(struct file *f, struct kstat *st);
static int fat32_write(struct file *f, const void *buf, size_t size);

struct file_operations *get_fat32_fops(void);
static void utf16le_to_ascii(char *dst, size_t dst_sz,
                             const uint16_t *src, size_t src_len);
static void lfn_clear(char *lfn_buf);
static void lfn_prepend_chunk(char *lfn_buf, size_t buf_sz,
                              const uint16_t *name1, size_t n1,
                              const uint16_t *name2, size_t n2,
                              const uint16_t *name3, size_t n3);