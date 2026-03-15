// fat32.c
#include "fat32.h"
#include "block.h"
#include "debug.h"
#include "string.h"

#define MAX_CLUSTER_BYTES 8192

#pragma pack(push, 1)
typedef struct {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  ntres;
    uint8_t  crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t fstClusHI;
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t fstClusLO;
    uint32_t fileSize;
} fat32_dirent_disk_t;

typedef struct {
    uint8_t  boot_flag;
    uint8_t  chs_start[3];
    uint8_t  type;
    uint8_t  chs_end[3];
    uint32_t lba_start;
    uint32_t sectors_total;
} mbr_part_t;
#pragma pack(pop)

// Forward declarations
static int read_sector(fat32_t *fs, uint32_t fs_lba, void *buf);

// Cluster -> FS-relative LBA
static uint32_t fat32_cluster_to_lba(fat32_t *fs, uint32_t cluster)
{
    if (cluster < 2) return 0;
    return fs->data_start_lba + (cluster - 2) * fs->sectors_per_cluster;
}

static int read_sector(fat32_t *fs, uint32_t fs_lba, void *buf)
{
    if (!fs || !fs->bdev || !fs->bdev->read_sectors || !buf)
        return -1;

    // fs_lba is filesystem-relative; device adds its lba_base
    return fs->bdev->read_sectors(fs->bdev,
                                  fs->bdev->lba_base + fs_lba,
                                  1, buf);
}

// Read whole cluster
static int read_cluster(fat32_t *fs, uint32_t cluster, void *buf)
{
    if (!fs || !fs->bdev || !buf) return -1;
    if (cluster < 2) return -1;

    uint32_t secs = fs->sectors_per_cluster;
    uint32_t fs_lba = fat32_cluster_to_lba(fs, cluster);
    if (fs_lba == 0) return -1;

    uint8_t *p = (uint8_t*)buf;
    for (uint32_t s = 0; s < secs; s++) {
        if (read_sector(fs, fs_lba + s, p + s * fs->bytes_per_sector) != 0) {
            log_error("FAT32", "read_cluster: read_sector failed cluster=%u fs_lba=%u",
                      cluster, fs_lba + s);
            return -1;
        }
    }
    return 0;
}

static uint32_t fat_next_cluster(fat32_t *fs, uint32_t cluster)
{
    if (!fs) return 0x0FFFFFFF;

    uint32_t fat_offset       = cluster * 4;
    uint32_t bytes_per_sector = fs->bytes_per_sector;
    // log_error("FAT32", "bytes_per_sector =%u", bytes_per_sector);

    uint32_t fs_fat_lba       = fs->fat_start_lba + (fat_offset / bytes_per_sector);
    uint32_t offset_in_sector = fat_offset % bytes_per_sector;

    uint8_t sec[512];
    if (read_sector(fs, fs_fat_lba, sec) != 0) {
        log_error("FAT32", "fat_next_cluster: read_sector failed fs_fat_lba=%u", fs_fat_lba);
        return 0x0FFFFFFF;
    }

    uint32_t entry;
    memcpy(&entry, sec + offset_in_sector, sizeof(entry));
    entry &= 0x0FFFFFFF;
    return entry;
}

static int is_eoc(uint32_t cl)
{
    cl &= 0x0FFFFFFF;
    return cl >= 0x0FFFFFF8;
}

// -----------------------------
// BPB parsing at given FS LBA
// -----------------------------
static int fat32_parse_bpb(fat32_t *fs, uint32_t vbr_fs_lba)
{
    uint8_t sec[512];

    if (read_sector(fs, vbr_fs_lba, sec) != 0) {
        log_error("FAT32", "BPB read failed");
        return -1;
    }

    if (sec[510] != 0x55 || sec[511] != 0xAA) {
        log_error("FAT32", "Invalid VBR signature");
        return -1;
    }

    uint16_t bytes_per_sector     = *(uint16_t*)&sec[11];
    uint8_t  sectors_per_cluster  = sec[13];
    uint16_t reserved_sectors     = *(uint16_t*)&sec[14];
    uint8_t  num_fats             = sec[16];
    uint16_t total_sectors16      = *(uint16_t*)&sec[19];
    uint16_t sectors_per_fat16    = *(uint16_t*)&sec[22];
    uint32_t total_sectors32      = *(uint32_t*)&sec[32];
    uint32_t sectors_per_fat32    = *(uint32_t*)&sec[36];
    uint32_t root_cluster         = *(uint32_t*)&sec[44];

    uint32_t total_sectors = total_sectors32 ? total_sectors32 : total_sectors16;
    uint32_t sectors_per_fat = sectors_per_fat32 ? sectors_per_fat32 : sectors_per_fat16;

    log_info("FAT32", "BPB: bytes/sec=%u spc=%u rsv=%u fats=%u tot=%u spf=%u rootClus=%u",
             bytes_per_sector, sectors_per_cluster, reserved_sectors,
             num_fats, total_sectors, sectors_per_fat, root_cluster);

    if (bytes_per_sector != 512 && bytes_per_sector != 1024 &&
        bytes_per_sector != 2048 && bytes_per_sector != 4096) {
        log_error("FAT32", "Unsupported bytes_per_sector=%u", bytes_per_sector);
        return -1;
    }
    if (sectors_per_cluster == 0 || (sectors_per_cluster & (sectors_per_cluster - 1)) != 0) {
        log_error("FAT32", "Invalid sectors_per_cluster=%u", sectors_per_cluster);
        return -1;
    }
    if (num_fats < 1 || num_fats > 2) {
        log_error("FAT32", "Unexpected num_fats=%u", num_fats);
        return -1;
    }
    if (sectors_per_fat == 0) {
        log_error("FAT32", "sectors_per_fat is zero");
        return -1;
    }
    if (root_cluster < 2) {
        log_error("FAT32", "root_cluster=%u invalid", root_cluster);
        return -1;
    }

    fs->bytes_per_sector    = bytes_per_sector;
    fs->sectors_per_cluster = sectors_per_cluster;
    fs->reserved_sectors    = reserved_sectors;
    fs->num_fats            = num_fats;
    fs->sectors_per_fat     = sectors_per_fat;
    fs->root_cluster        = root_cluster;

    // FS-relative LBAs
    fs->fat_start_lba  = reserved_sectors;
    fs->data_start_lba = reserved_sectors + num_fats * sectors_per_fat;

    log_info("FAT32", "Layout: fat_start=%u data_start=%u", fs->fat_start_lba, fs->data_start_lba);
    return 0;
}

// -----------------------------
// FAT32 init on block device
// -----------------------------
fat32_t* fat32_init_device(block_device_t *bdev)
{
    if (!bdev) {
        log_error("FAT32", "fat32_init_device: bdev=NULL");
        return NULL;
    }

    fat32_t *fs = (fat32_t*)kmalloc(sizeof(fat32_t));
    if (!fs) {
        log_error("FAT32", "fat32_init_device: fs=NULL");
        return NULL;
    }
    memset(fs, 0, sizeof(*fs));
    fs->bdev = bdev;

    // If lba_base != 0, bdev is already a partition device
    // FS starts at FS-relative LBA 0 on that device
    if (bdev->lba_base != 0) {
        log_info("FAT32", "Init on partition device '%s' lba_base=%u",
                 bdev->name, bdev->lba_base);

        if (fat32_parse_bpb(fs, 0) != 0) {
            log_error("FAT32", "Failed to parse VBR on partition");
            kfree(fs);
            return NULL;
        }
        return fs;
    }

    // Otherwise, raw disk: read MBR, find FAT32 partition, parse its VBR
    uint8_t mbr[512];
    if (read_sector(fs, 0, mbr) != 0) {
        log_error("FAT32", "Failed to read MBR on raw disk");
        kfree(fs);
        return NULL;
    }

    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        log_error("FAT32", "Invalid MBR signature on raw disk");
        kfree(fs);
        return NULL;
    }

    mbr_part_t *part = (mbr_part_t*)&mbr[0x1BE];
    int found = 0;
    uint32_t fat32_start_lba = 0;

    for (int i = 0; i < 4; i++, part++) {
        if (part->type == 0x0B || part->type == 0x0C) {
            found = 1;
            fat32_start_lba = part->lba_start;
            log_info("FAT32", "Found FAT32 partition in MBR: lba_start=%u (entry %d)",
                     fat32_start_lba, i);
            break;
        }
    }

    if (!found) {
        log_error("FAT32", "No FAT32 partition in MBR");
        kfree(fs);
        return NULL;
    }

    // For raw disk init, we interpret vbr_fs_lba = fat32_start_lba,
    // but read_sector will still add bdev->lba_base (0). To keep FS-relative
    // semantics inside fs, we store an offset in the bdev->lba_base externally
    // instead of offsetting fs->fields. However, since bdev belongs to caller,
    // it's better you call this on a partition device (sda1) in practice.
    if (fat32_parse_bpb(fs, fat32_start_lba) != 0) {
        log_error("FAT32", "Failed to parse FAT32 VBR at LBA %u", fat32_start_lba);
        kfree(fs);
        return NULL;
    }

    return fs;
}

// -----------------------------
// Path lookup, open/read/readdir
// (largely as you had, but cleaned)
// -----------------------------

static void fat32_make_83(const char *input, uint8_t out[11])
{
    memset(out, ' ', 11);

    int i = 0;
    int pos = 0;

    while (input[i] && input[i] != '.' && pos < 8) {
        char c = input[i++];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[pos++] = c;
    }

    if (input[i] == '.') {
        i++;
        pos = 8;
        int ext_i = 0;
        while (input[i] && ext_i < 3) {
            char c = input[i++];
            if (c >= 'a' && c <= 'z') c -= 32;
            out[pos++] = c;
            ext_i++;
        }
    }
}

static void make_short_name(const fat32_dirent_disk_t *d, char out[NAME_MAX])
{
    char name[9];
    char ext[4];

    memcpy(name, d->name, 8); name[8] = '\0';
    memcpy(ext,  d->name+8, 3); ext[3] = '\0';

    int ni = 7; while (ni >= 0 && name[ni] == ' ') name[ni--] = '\0';
    int ei = 2; while (ei >= 0 && ext[ei]  == ' ') ext[ei--] = '\0';

    for (int i = 0; name[i]; i++)
        if (name[i] >= 'a' && name[i] <= 'z') name[i] -= 32;
    for (int i = 0; ext[i]; i++)
        if (ext[i] >= 'a' && ext[i] <= 'z') ext[i] -= 32;

    size_t pos = 0;
    size_t max = NAME_MAX;

    for (size_t i = 0; name[i] && pos < max-1; i++)
        out[pos++] = name[i];

    if (ext[0] && pos < max-1) {
        out[pos++] = '.';
        for (size_t i = 0; ext[i] && pos < max-1; i++)
            out[pos++] = ext[i];
    }

    out[pos] = '\0';
}

int lookup_path(fat32_t *fs, const char *path, fat32_node_t *out)
{
    if (!fs || !out || !path) return -1;

    const char *p = path;
    if (*p == '/') p++;

    if (*p == '\0') {
        memset(out, 0, sizeof(*out));
        out->fs            = fs;
        out->first_cluster = fs->root_cluster;
        out->is_dir        = 1;
        strncpy(out->name, "/", NAME_MAX);
        return 0;
    }

    uint32_t cluster_bytes = fs->bytes_per_sector * fs->sectors_per_cluster;
    if (cluster_bytes > MAX_CLUSTER_BYTES)
        return -1;

    uint8_t cluster_buf[MAX_CLUSTER_BYTES];
    uint32_t current_cluster = fs->root_cluster;

    char component[NAME_MAX];

    while (*p) {
        int ci = 0;
        while (*p && *p != '/' && ci < NAME_MAX-1)
            component[ci++] = *p++;
        component[ci] = '\0';
        if (*p == '/') p++;

        if (ci == 0)
            continue;

        uint8_t want83[11];
        fat32_make_83(component, want83);

        int found = 0;
        uint32_t cl = current_cluster;

        while (!is_eoc(cl)) {
            uint32_t fs_lba = fat32_cluster_to_lba(fs, cl);

            for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
                if (read_sector(fs, fs_lba + s, cluster_buf) != 0)
                    return -1;

                uint32_t entries = cluster_bytes / 32;
                for (uint32_t e = 0; e < entries; e++) {
                    fat32_dirent_disk_t *de =
                        (fat32_dirent_disk_t*)(cluster_buf + e*32);

                    if (de->name[0] == 0x00)
                        goto search_fail;

                    if (de->name[0] == 0xE5) continue;
                    if ((de->attr & 0x0F) == 0x0F) continue;

                    if (memcmp(de->name, want83, 11) == 0) {
                        uint32_t first =
                            ((uint32_t)de->fstClusHI << 16) |
                             (uint32_t)de->fstClusLO;

                        memset(out, 0, sizeof(*out));
                        out->fs            = fs;
                        out->first_cluster = first;
                        out->cur_cluster   = first;
                        out->size          = de->fileSize;
                        out->is_dir        = (de->attr & 0x10) != 0;
                        strncpy(out->name, component, NAME_MAX);
                        found = 1;
                        goto search_done;
                    }
                }
            }

search_fail:
            cl = fat_next_cluster(fs, cl);
        }

search_done:
        if (!found)
            return -1;

        if (*p == '\0')
            return 0;

        if (!out->is_dir)
            return -1;

        current_cluster = out->first_cluster;
    }

    return -1;
}

// file_operations

static int fat32_open(struct file *f, int flags)
{
    fat32_t *fs = (fat32_t*)f->private_data;
    if (!fs) return -1;

    fat32_node_t *node = (fat32_node_t*)kmalloc(sizeof(fat32_node_t));
    if (!node) return -1;
    memset(node, 0, sizeof(*node));

    if (f->subpath && f->subpath[0] != '\0') {
        if (lookup_path(fs, f->subpath, node) != 0) {
            kfree(node);
            return -1;
        }
    } else {
        node->fs            = fs;
        node->first_cluster = fs->root_cluster;
        node->cur_cluster   = fs->root_cluster;
        node->size          = 0;
        node->is_dir        = 1;
        strcpy(node->name, "/");
    }

    node->cur_offset = 0;
    f->private_data  = node;
    return 0;
}

static int fat32_close(struct file *f)
{
    if (f && f->private_data) {
        kfree(f->private_data);
        f->private_data = NULL;
    }
    return 0;
}

static int fat32_readdir(struct file *f, dirent_t *e)
{
    fat32_node_t *node = (fat32_node_t*)f->private_data;
    if (!node || !node->is_dir) return -1;

    uint32_t cur = node->cur_cluster;
    uint8_t cluster_buf[MAX_CLUSTER_BYTES];

    while (!is_eoc(cur)) {
        if (read_cluster(node->fs, cur, cluster_buf) != 0)
            return -1;

        size_t cluster_bytes = node->fs->sectors_per_cluster *
                               node->fs->bytes_per_sector;

        while (node->cur_offset < cluster_bytes) {
            fat32_dirent_disk_t *d =
                (fat32_dirent_disk_t*)(cluster_buf + node->cur_offset);
            node->cur_offset += sizeof(fat32_dirent_disk_t);

            if (d->name[0] == 0x00) return -1;
            if (d->name[0] == 0xE5) continue;
            //if ((d->attr & 0x0F) == 0x0F) continue;

            make_short_name(d, e->name);
            e->inode = 0;
            e->type  = (d->attr & 0x10) ? 1 : 0;
            return 0;
        }

        uint32_t next = fat_next_cluster(node->fs, cur);
        node->cur_cluster = cur = next;
        node->cur_offset  = 0;
    }

    return -1;
}

static int fat32_read(struct file *f, void *buf, size_t size)
{
    fat32_node_t *node = (fat32_node_t*)f->private_data;
    if (!node || node->is_dir) return -1;

    size_t remaining  = size;
    size_t read_total = 0;
    size_t file_pos   = f->position;

    uint32_t cluster_size = node->fs->bytes_per_sector *
                            node->fs->sectors_per_cluster;
    uint32_t cluster = node->first_cluster;

    size_t skip = file_pos;
    while (skip >= cluster_size && !is_eoc(cluster)) {
        cluster = fat_next_cluster(node->fs, cluster);
        skip   -= cluster_size;
    }
    if (is_eoc(cluster))
        return 0;

    uint8_t clbuf[MAX_CLUSTER_BYTES];

    while (remaining > 0 && !is_eoc(cluster)) {
        if (read_cluster(node->fs, cluster, clbuf) != 0)
            break;

        size_t chunk = cluster_size - skip;
        if (chunk > remaining) chunk = remaining;

        memcpy((uint8_t*)buf + read_total, clbuf + skip, chunk);
        read_total += chunk;
        remaining  -= chunk;
        skip        = 0;

        cluster = fat_next_cluster(node->fs, cluster);
    }

    f->position += read_total;
    if (f->position > node->size)
        f->position = node->size;

    return (int)read_total;
}

static int fat_stat(struct file *f, struct kstat *st)
{
    fat32_node_t *node = (fat32_node_t *)f->private_data;
    if (!node)
        return -1;

    st->st_dev   = 1;                     // single FAT32 device for now
    st->st_ino   = node->first_cluster;   // good inode surrogate
    st->st_mode  = node->is_dir ? 0x4000  // dir
                                : 0x8000; // regular file
    st->st_nlink = 1;
    st->st_size  = node->size;

    //log_info("FAT32", "fat_stat: size=%u first_cluster=%u",
    //     node->size, node->first_cluster);

    return 0;
}

static struct file_operations fat32_fops = {
    .open    = fat32_open,
    .close   = fat32_close,
    .read    = fat32_read,
    .write   = NULL,
    .ioctl   = NULL,
    .readdir = fat32_readdir,
    .stat    = fat_stat,
};

struct file_operations *get_fat32_fops(void)
{
    return &fat32_fops;
}
