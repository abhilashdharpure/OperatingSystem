// fat32.c
#include "fat32.h"
#include "block.h"
#include "debug.h"
#include "string.h"

#define MAX_CLUSTER_BYTES 8192
#define MAX_SLOTS 22

#include "fcntl.h"
#ifndef O_ACCMODE
#define O_ACCMODE 3
#endif

#ifndef O_DIRECTORY
#define O_DIRECTORY 0200000        /* 0x10000, the Linux value */
#endif

static int  write_sector(fat32_t *fs, uint32_t fs_lba, const void *buf);
static void fat_free_chain(fat32_t *fs, uint32_t cl);

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
    uint8_t  ord;          // sequence number
    uint16_t name1[5];     // 1–5 chars
    uint8_t  attr;         // 0x0F
    uint8_t  type;
    uint8_t  checksum;
    uint16_t name2[6];     // 6–11 chars
    uint16_t fstClusLO;
    uint16_t name3[2];     // 12–13 chars
} fat32_lfn_entry_t;

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

    uint32_t fat_offset = cluster * 4;
    uint32_t lba        = fs->fat_start_lba + (fat_offset / fs->bytes_per_sector);
    uint32_t off        = fat_offset % fs->bytes_per_sector;

    if (fs->fat_cache_lba != lba) {
        if (read_sector(fs, lba, fs->fat_cache) != 0) {
            fs->fat_cache_lba = 0xFFFFFFFF;
            log_error("FAT32", "fat_next_cluster: read failed lba=%u", lba);
            return 0x0FFFFFFF;
        }
        fs->fat_cache_lba = lba;
    }
    uint32_t entry;
    memcpy(&entry, fs->fat_cache + off, sizeof(entry));
    return entry & 0x0FFFFFFF;
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

    if (bytes_per_sector != 512)
    {
        log_error("FAT32", "only 512-byte sectors supported");
        return -1;
    }

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

    fs->total_clusters = (total_sectors - fs->data_start_lba) / sectors_per_cluster;
    fs->next_free = 2;

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
    fs->fat_cache_lba = 0xFFFFFFFF;
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

    for (int i = 0; name[i]; i++) if (name[i] >= 'a' && name[i] <= 'z') name[i] -= 32;
    for (int i = 0; ext[i];  i++) if (ext[i]  >= 'a' && ext[i]  <= 'z') ext[i]  -= 32;

    if (d->ntres & 0x08) for (int i = 0; name[i]; i++) if (name[i] >= 'A' && name[i] <= 'Z') name[i] += 32;
    if (d->ntres & 0x10) for (int i = 0; ext[i];  i++) if (ext[i]  >= 'A' && ext[i]  <= 'Z') ext[i]  += 32;

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
    if (!fs || !out || !path)
    {
        log_info("FAT32", "lookup_path returnung -1, from exit 1");
        return -1;
    }

    const char *p = path;
    if (*p == '/') p++;

    // Root directory
    if (*p == '\0') {
        memset(out, 0, sizeof(*out));
        out->fs            = fs;
        out->first_cluster = fs->root_cluster;
        out->cur_cluster   = fs->root_cluster;
        out->is_dir        = 1;
        strcpy(out->name, "/");
        return 0;
    }

    uint32_t cluster_bytes = fs->bytes_per_sector * fs->sectors_per_cluster;
    if (cluster_bytes > MAX_CLUSTER_BYTES)
    {
        log_info("FAT32", "lookup_path returnung -1, from exit 2");
        return -1;
    }

    uint8_t cluster_buf[MAX_CLUSTER_BYTES];
    uint32_t current_cluster = fs->root_cluster;

    char component[NAME_MAX];

    while (*p)
    {
        /* Extract next path component */
        int ci = 0;
        while (*p && *p != '/' && ci < NAME_MAX - 1)
            component[ci++] = *p++;
        component[ci] = '\0';
        if (*p == '/') p++;

        if (ci == 0)
            continue;

        /* Prepare for scanning directory */
        uint32_t cl = current_cluster;
        int found = 0;

        char lfn[NAME_MAX];
        lfn[0] = '\0';

        while (!is_eoc(cl))
        {
            uint32_t fs_lba = fat32_cluster_to_lba(fs, cl);

            for (uint32_t s = 0; s < fs->sectors_per_cluster; s++)
            {
                if (read_sector(fs, fs_lba + s, cluster_buf) != 0)
                {
                    log_info("FAT32", "lookup_path returnung -1, from exit 3");
                    return -1;
                }

                uint32_t entries = fs->bytes_per_sector / 32;

                for (uint32_t e = 0; e < entries; e++)
                {
                    uint8_t *pde = cluster_buf + e * 32;
                    fat32_dirent_disk_t *de = (fat32_dirent_disk_t*)pde;

                    /* End of directory */
                    if (de->name[0] == 0x00)
                        goto search_fail;

                    /* Deleted entry */
                    if (de->name[0] == 0xE5) {
                        lfn[0] = '\0';
                        continue;
                    }

                    /* LFN entry */
                    if ((de->attr & 0x0F) == 0x0F)
                    {
                        fat32_lfn_entry_t *lf = (fat32_lfn_entry_t*)pde;

                        if (lf->ord & 0x40)   // start of LFN sequence
                            lfn[0] = '\0';

                        // Append this LFN chunk
                        char chunk[NAME_MAX];
                        chunk[0] = '\0';

                        utf16le_to_ascii(chunk, sizeof(chunk), lf->name1, 5);
                        utf16le_to_ascii(chunk, sizeof(chunk), lf->name2, 6);
                        utf16le_to_ascii(chunk, sizeof(chunk), lf->name3, 2);

                        // Prepend chunk
                        char tmp[NAME_MAX];
                        strcpy(tmp, lfn);
                        snprintf(lfn, sizeof(lfn), "%s%s", chunk, tmp);

                        continue;
                    }

                    /* Short entry */
                    char shortname[NAME_MAX];
                    make_short_name(de, shortname);

                    /* Compare component with long name or short name */
                    int match = 0;

                    if (lfn[0] != '\0') {
                        if (strcasecmp(component, lfn) == 0)
                            match = 1;
                    }

                    if (!match) {
                        if (strcasecmp(component, shortname) == 0)
                            match = 1;
                    }

                    if (match)
                    {
                        uint32_t first =
                            ((uint32_t)de->fstClusHI << 16) |
                             (uint32_t)de->fstClusLO;

                        memset(out, 0, sizeof(*out));
                        out->fs            = fs;
                        out->first_cluster = first;
                        out->cur_cluster   = first;
                        out->size          = de->fileSize;
                        out->is_dir        = (de->attr & 0x10) != 0;
                        out->dirent_lba     = fs_lba + s;
                        out->dirent_off     = (uint16_t)(e * 32);
                        out->parent_cluster = current_cluster;

                        strncpy(out->name, (lfn[0] ? lfn : shortname), NAME_MAX);

                        found = 1;
                        goto search_done;
                    }

                    /* Reset LFN buffer after consuming short entry */
                    lfn[0] = '\0';
                }
            }

            cl = fat_next_cluster(fs, cl);
        }

search_fail:
        if (!found)
        {
            log_info("FAT32", "lookup_path returnung -1, from exit 4");
            return -1;
        }

search_done:
        if (*p == '\0')
            return 0;

        if (!out->is_dir)
        {
            log_info("FAT32", "lookup_path returnung -1, from exit 5");
            return -1;
        }

        current_cluster = out->first_cluster;
    }


    log_info("FAT32", "lookup_path returnung -1, from exit 6");
    return -1;
}
static int node_flush(fat32_node_t *n)                  // size and first cluster back to the directory
{
    if (!n->dirent_lba) return 0;
    uint8_t sec[512];
    if (read_sector(n->fs, n->dirent_lba, sec) != 0) return -1;
    fat32_dirent_disk_t *d = (fat32_dirent_disk_t *)(sec + n->dirent_off);
    d->fileSize  = n->size;
    d->fstClusHI = (uint16_t)(n->first_cluster >> 16);
    d->fstClusLO = (uint16_t)(n->first_cluster & 0xFFFF);
    return write_sector(n->fs, n->dirent_lba, sec);
}

static int node_truncate(fat32_node_t *n)
{
    uint32_t old = n->first_cluster;
    n->first_cluster = n->cur_cluster = 0;
    n->size = 0;
    n->seek_cluster = 0; n->seek_index = 0;
    if (node_flush(n) != 0) {                        // could not update the entry: change nothing
        n->first_cluster = n->cur_cluster = old;
        return -1;
    }
    if (old >= 2) fat_free_chain(n->fs, old);
    return 0;
}


static int fat32_open(struct file *f, int flags)
{
    fat32_t *fs = (fat32_t *)f->private_data;
    if (!fs) return -1;

    fat32_node_t *node = (fat32_node_t *)kmalloc(sizeof(fat32_node_t));
    if (!node) return -1;
    memset(node, 0, sizeof(*node));



    if (f->subpath && f->subpath[0] != '\0') {
        if (lookup_path(fs, f->subpath, node) == 0) {
            if ((flags & O_CREAT) && (flags & O_EXCL))
            {
                kfree(node);
                return -1;
            }
            if ((flags & O_TRUNC) && (flags & O_ACCMODE) && !node->is_dir)
            {
                if (node_truncate(node) != 0)
                {
                    kfree(node);
                    return -1;
                }
            }
        }
        else
        {
            if (!(flags & O_CREAT) || fat32_create_file(fs, f->subpath, node) != 0)
            {
                kfree(node);
                return -1;
            }
        }
    } else {
        node->fs = fs;
        node->first_cluster = fs->root_cluster;
        node->cur_cluster = fs->root_cluster;
        node->is_dir = 1;
        strcpy(node->name, "/");
    }

    if ((flags & O_DIRECTORY) && !node->is_dir)
    {
        kfree(node);
        return -1;
    }   // ENOTDIR
    if (node->is_dir && (flags & O_ACCMODE))
    {
        kfree(node);
        return -1;
    }   // EISDIR: no write access to folders

    node->cur_offset = 0;
    f->private_data = node;
    if (flags & O_APPEND) f->position = node->size;
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

    char lfn[NAME_MAX];
    lfn_clear(lfn);

    while (!is_eoc(cur)) {
        if (read_cluster(node->fs, cur, cluster_buf) != 0)
            return -1;

        size_t cluster_bytes = node->fs->sectors_per_cluster *
                               node->fs->bytes_per_sector;

        while (node->cur_offset < cluster_bytes) {
            uint8_t *p = cluster_buf + node->cur_offset;
            fat32_dirent_disk_t *d = (fat32_dirent_disk_t*)p;
            node->cur_offset += sizeof(fat32_dirent_disk_t);

            // End of directory
            if (d->name[0] == 0x00) {
                return -1;
            }

            // Deleted
            if (d->name[0] == 0xE5) {
                lfn_clear(lfn);
                continue;
            }

            // LFN entry?
            if ((d->attr & 0x0F) == 0x0F) {
                fat32_lfn_entry_t *lfn_ent = (fat32_lfn_entry_t*)p;

                // If this is the start of a new LFN sequence, clear buffer
                if (lfn_ent->ord & 0x40) {
                    lfn_clear(lfn);
                }

                // Prepend this chunk (we build from last to first)
                lfn_prepend_chunk(lfn, sizeof(lfn),
                                  lfn_ent->name1, 5,
                                  lfn_ent->name2, 6,
                                  lfn_ent->name3, 2);
                continue;
            }

            // Normal 8.3 entry
            if (d->attr == 0x00) {
                lfn_clear(lfn);
                continue;
            }

            if (d->attr & 0x08) { lfn_clear(lfn); continue; }

            // Skip volume labels, etc. if you want:
            // if (d->attr & 0x08) { lfn_clear(lfn); continue; }

            // Decide which name to use
            if (lfn[0] != '\0') {
                strncpy(e->name, lfn, NAME_MAX);
                e->name[NAME_MAX-1] = '\0';
            } else {
                make_short_name(d, e->name);
            }

            e->inode = 0;
            e->type  = (d->attr & 0x10) ? 1 : 0;

            // LFN consumed
            lfn_clear(lfn);
            return 0;
        }

        uint32_t next = fat_next_cluster(node->fs, cur);
        node->cur_cluster = cur = next;
        node->cur_offset  = 0;
    }

    return -1;
}

static int read_sectors_fs(fat32_t *fs, uint32_t fs_lba, uint32_t count, void *buf)
{
    return fs->bdev->read_sectors(fs->bdev, fs->bdev->lba_base + fs_lba, count, buf);
}


typedef struct { uint32_t lba; uint16_t off; } slot_t;

static int write_sectors_fs(fat32_t *fs, uint32_t fs_lba, uint32_t count, const void *buf)
{
    if (!fs || !fs->bdev || !fs->bdev->write_sectors)
    {
        log_error("FAT32", "write_sectors missing on bdev '%s'", fs && fs->bdev ? fs->bdev->name : "?");
        return -1;
    }
    return fs->bdev->write_sectors(fs->bdev, fs->bdev->lba_base + fs_lba, count, buf);
}
static int write_sector(fat32_t *fs, uint32_t fs_lba, const void *buf)
{
    return write_sectors_fs(fs, fs_lba, 1, buf);
}

static int write_cluster(fat32_t *fs, uint32_t cluster, const void *buf)
{
    if (!fs || !buf || cluster < 2)
        return -1;

    uint32_t fs_lba = fat32_cluster_to_lba(fs, cluster);

    const uint8_t *p = (const uint8_t *)buf;

    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++)
    {
        if (write_sector(
                fs,
                fs_lba + s,
                p + s * fs->bytes_per_sector) != 0)
        {
            return -1;
        }
    }

    return 0;
}

static int fat32_read(struct file *f, void *buf, size_t size)
{
    fat32_node_t *node = (fat32_node_t *)f->private_data;
    if (!node || node->is_dir || !buf) return -1;
    if (f->position >= node->size) return 0;

    size_t left = node->size - f->position;
    if (size > left) size = left;
    if (size == 0) return 0;

    fat32_t *fs = node->fs;
    uint32_t bps = fs->bytes_per_sector;
    uint32_t cluster_size = bps * fs->sectors_per_cluster;
    if (cluster_size == 0 || cluster_size > MAX_CLUSTER_BYTES) return -1;

    uint32_t target = (uint32_t)(f->position / cluster_size);
    uint32_t skip   = (uint32_t)(f->position % cluster_size);

    /* start from the cached position when it is not past the target */
    uint32_t cluster, idx;
    if (node->seek_cluster >= 2 && node->seek_index <= target) {
        cluster = node->seek_cluster;  idx = node->seek_index;
    } else {
        cluster = node->first_cluster; idx = 0;
    }
    while (idx < target) {
        if (cluster < 2 || is_eoc(cluster)) return 0;
        cluster = fat_next_cluster(fs, cluster);
        idx++;
    }

    uint8_t  bounce[MAX_CLUSTER_BYTES];
    uint8_t *out  = (uint8_t *)buf;
    size_t   done = 0;

    while (done < size && cluster >= 2 && !is_eoc(cluster)) {
        node->seek_cluster = cluster;
        node->seek_index   = idx;

        size_t chunk = cluster_size - skip;
        if (chunk > size - done) chunk = size - done;

        uint32_t lba   = fat32_cluster_to_lba(fs, cluster);
        uint32_t first = skip / bps;
        uint32_t off   = skip % bps;

        if (off == 0 && (chunk % bps) == 0) {
            /* aligned: read straight into the caller's buffer */
            if (read_sectors_fs(fs, lba + first, (uint32_t)(chunk / bps), out + done) != 0) break;
        } else {
            uint32_t nsec = (uint32_t)((off + chunk + bps - 1) / bps);
            if (read_sectors_fs(fs, lba + first, nsec, bounce) != 0) break;
            memcpy(out + done, bounce + off, chunk);
        }

        done += chunk;
        skip = 0;
        if (done < size) { cluster = fat_next_cluster(fs, cluster); idx++; }
    }

    f->position += done;
    return (int)done;
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
    .write   = fat32_write,
    .ioctl   = NULL,
    .readdir = fat32_readdir,
    .stat    = fat_stat,
};

struct file_operations *get_fat32_fops(void)
{
    return &fat32_fops;
}

static void utf16le_to_ascii(char *dst, size_t dst_sz,
                             const uint16_t *src, size_t src_len)
{
    size_t pos = strlen(dst);
    for (size_t i = 0; i < src_len && pos + 1 < dst_sz; ++i) {
        uint16_t ch = src[i];
        if (ch == 0x0000 || ch == 0xFFFF)
            break;
        // naive: drop high byte
        dst[pos++] = (char)(ch & 0xFF);
    }
    dst[pos] = '\0';
}

static void lfn_clear(char *lfn_buf)
{
    lfn_buf[0] = '\0';
}

static void lfn_prepend_chunk(char *lfn_buf, size_t buf_sz,
                              const uint16_t *name1, size_t n1,
                              const uint16_t *name2, size_t n2,
                              const uint16_t *name3, size_t n3)
{
    char chunk[NAME_MAX];
    chunk[0] = '\0';

    utf16le_to_ascii(chunk, sizeof(chunk), name1, n1);
    utf16le_to_ascii(chunk, sizeof(chunk), name2, n2);
    utf16le_to_ascii(chunk, sizeof(chunk), name3, n3);

    // prepend chunk to existing lfn_buf
    char tmp[NAME_MAX];
    strncpy(tmp, lfn_buf, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    snprintf(lfn_buf, buf_sz, "%s%s", chunk, tmp);
}

/* ================= write support ================= */


/* ---- FAT table ---- */
static int fat_set(fat32_t *fs, uint32_t cl, uint32_t val)
{
    uint32_t byte = cl * 4;
    uint32_t lba  = fs->fat_start_lba + byte / fs->bytes_per_sector;
    uint32_t off  = byte % fs->bytes_per_sector;

    if (fs->fat_cache_lba != lba) {                     // the cache holds one 512-byte FAT sector
        if (read_sector(fs, lba, fs->fat_cache) != 0) { fs->fat_cache_lba = 0xFFFFFFFF; return -1; }
        fs->fat_cache_lba = lba;
    }
    uint32_t old;
    memcpy(&old, fs->fat_cache + off, 4);
    val = (old & 0xF0000000u) | (val & 0x0FFFFFFFu);    // keep the 4 reserved bits
    memcpy(fs->fat_cache + off, &val, 4);

    for (uint32_t i = 0; i < fs->num_fats; i++)
        if (write_sector(fs, lba + i * fs->sectors_per_fat, fs->fat_cache) != 0) {
            fs->fat_cache_lba = 0xFFFFFFFF;          // memory no longer matches the disk
            return -1;
        }
    return 0;
}

static uint32_t fat_alloc(fat32_t *fs)                  // returns 0 when the disk is full
{
    uint32_t n = fs->total_clusters + 2;                // valid clusters: 2 .. n-1
    uint32_t start = (fs->next_free < 2 || fs->next_free >= n) ? 2 : fs->next_free;
    for (uint32_t k = 0; k < n - 2; k++) {
        uint32_t cl = start + k;
        if (cl >= n) cl -= (n - 2);
        if (fat_next_cluster(fs, cl) == 0) {
            if (fat_set(fs, cl, 0x0FFFFFFF) != 0) return 0;
            fs->next_free = cl + 1;
            return cl;
        }
    }
    return 0;
}

static void fat_free_chain(fat32_t *fs, uint32_t cl)
{
    while (cl >= 2 && !is_eoc(cl)) {
        uint32_t next = fat_next_cluster(fs, cl);
        fat_set(fs, cl, 0);
        if (cl < fs->next_free) fs->next_free = cl;
        cl = next;
    }
}

static int zero_cluster(fat32_t *fs, uint32_t cl)
{
    uint8_t z[512];
    memset(z, 0, fs->bytes_per_sector);
    uint32_t base = fat32_cluster_to_lba(fs, cl);
    for (uint32_t s = 0; s < fs->sectors_per_cluster; s++)
        if (write_sector(fs, base + s, z) != 0) return -1;
    return 0;
}

/* ---- directory entry helpers ---- */
static int put_entry(fat32_t *fs, slot_t s, const void *e32)
{
    uint8_t sec[512];
    if (read_sector(fs, s.lba, sec) != 0) return -1;
    memcpy(sec + s.off, e32, 32);
    return write_sector(fs, s.lba, sec);
}

static int mark_deleted(fat32_t *fs, slot_t s)
{
    uint8_t sec[512];
    if (read_sector(fs, s.lba, sec) != 0) return -1;
    sec[s.off] = 0xE5;
    return write_sector(fs, s.lba, sec);
}



/* find `need` consecutive free entries in a directory, extending it if necessary */
static int dir_alloc_slots(fat32_t *fs, uint32_t dir_cl, int need, slot_t *out)
{
    uint8_t sec[512];
    uint32_t bps = fs->bytes_per_sector, spc = fs->sectors_per_cluster;
    uint32_t cl = dir_cl, last = dir_cl;
    int run = 0;

    while (cl >= 2 && !is_eoc(cl)) {
        last = cl;
        uint32_t base = fat32_cluster_to_lba(fs, cl);
        for (uint32_t s = 0; s < spc; s++) {
            if (read_sector(fs, base + s, sec) != 0) return -1;
            for (uint32_t e = 0; e < bps / 32; e++) {
                uint8_t b0 = sec[e * 32];
                if (b0 == 0x00 || b0 == 0xE5) {
                    out[run].lba = base + s; out[run].off = (uint16_t)(e * 32);
                    if (++run == need) return 0;
                } else run = 0;
            }
        }
        cl = fat_next_cluster(fs, cl);
    }
    while (run < need) {                                // directory is full: add a cluster
        uint32_t nc = fat_alloc(fs);
        if (!nc || zero_cluster(fs, nc) != 0 || fat_set(fs, last, nc) != 0) return -1;
        last = nc;
        uint32_t base = fat32_cluster_to_lba(fs, nc);
        for (uint32_t s = 0; s < spc && run < need; s++)
            for (uint32_t e = 0; e < bps / 32 && run < need; e++) {
                out[run].lba = base + s; out[run].off = (uint16_t)(e * 32); run++;
            }
    }
    return 0;
}

static int dir_has_sfn(fat32_t *fs, uint32_t dir_cl, const uint8_t *sfn)
{
    uint8_t sec[512];
    for (uint32_t cl = dir_cl; cl >= 2 && !is_eoc(cl); cl = fat_next_cluster(fs, cl)) {
        uint32_t base = fat32_cluster_to_lba(fs, cl);
        for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
            if (read_sector(fs, base + s, sec) != 0) return 0;
            for (uint32_t e = 0; e < fs->bytes_per_sector / 32; e++) {
                uint8_t *p = sec + e * 32;
                if (p[0] == 0x00) return 0;
                if (p[0] == 0xE5 || (p[11] & 0x0F) == 0x0F) continue;
                if (memcmp(p, sfn, 11) == 0) return 1;
            }
        }
    }
    return 0;
}

static int dir_is_empty(fat32_t *fs, uint32_t dir_cl)
{
    uint8_t sec[512];
    for (uint32_t cl = dir_cl; cl >= 2 && !is_eoc(cl); cl = fat_next_cluster(fs, cl)) {
        uint32_t base = fat32_cluster_to_lba(fs, cl);
        for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
            if (read_sector(fs, base + s, sec) != 0) return 0;
            for (uint32_t e = 0; e < fs->bytes_per_sector / 32; e++) {
                uint8_t *p = sec + e * 32;
                if (p[0] == 0x00) return 1;
                if (p[0] == 0xE5 || (p[11] & 0x0F) == 0x0F || p[0] == '.') continue;
                return 0;
            }
        }
    }
    return 1;
}

/* delete the 8.3 entry at (t_lba, t_off) together with its long-name entries */
static int dir_delete_entry(fat32_t *fs, uint32_t dir_cl, uint32_t t_lba, uint16_t t_off)
{
    uint8_t sec[512];
    slot_t pend[MAX_SLOTS];
    int np = 0;
    for (uint32_t cl = dir_cl; cl >= 2 && !is_eoc(cl); cl = fat_next_cluster(fs, cl)) {
        uint32_t base = fat32_cluster_to_lba(fs, cl);
        for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
            if (read_sector(fs, base + s, sec) != 0) return -1;
            for (uint32_t e = 0; e < fs->bytes_per_sector / 32; e++) {
                uint8_t *p = sec + e * 32;
                if (p[0] == 0x00) return -1;
                if (p[0] == 0xE5) { np = 0; continue; }
                if ((p[11] & 0x0F) == 0x0F) {
                    if (np < MAX_SLOTS - 1) { pend[np].lba = base + s; pend[np].off = (uint16_t)(e * 32); np++; }
                    continue;
                }
                if (base + s == t_lba && e * 32 == t_off) {
                    for (int i = 0; i < np; i++) if (mark_deleted(fs, pend[i]) != 0) return -1;
                    slot_t self = { t_lba, t_off };
                    return mark_deleted(fs, self);
                }
                np = 0;
            }
        }
    }
    return -1;
}

/* ---- names ---- */
static int is_sfn_char(char c)
{
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) return 1;
    static const char ok[] = "$%'-_@~`!(){}^#&";
    for (const char *p = ok; *p; p++) if (*p == c) return 1;
    return 0;
}

static uint8_t lfn_checksum(const uint8_t *n11)
{
    uint8_t s = 0;
    for (int i = 0; i < 11; i++) s = (uint8_t)(((s & 1) ? 0x80 : 0) + (s >> 1) + n11[i]);
    return s;
}

typedef struct { uint8_t sfn[11]; uint8_t ntres; int lfn; } name_plan_t;

static char up(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }

static int plan_name(fat32_t *fs, uint32_t dir_cl, const char *name, name_plan_t *p)
{
    size_t len = strlen(name);
    if (len == 0 || len > 255) return -1;
    if ((len == 1 && name[0] == '.') || (len == 2 && name[0] == '.' && name[1] == '.')) return -1;

    int dot = -1;
    for (size_t i = 0; i < len; i++) if (name[i] == '.') dot = (int)i;
    size_t blen = dot > 0 ? (size_t)dot : len;
    const char *ext = dot > 0 ? name + dot + 1 : "";
    size_t elen = strlen(ext);

    int fits = blen >= 1 && blen <= 8 && elen <= 3;
    int b_lo = 0, b_up = 0, e_lo = 0, e_up = 0;
    for (size_t i = 0; fits && i < blen; i++) {
        if (!is_sfn_char(name[i])) fits = 0;
        if (name[i] >= 'a' && name[i] <= 'z') b_lo = 1;
        if (name[i] >= 'A' && name[i] <= 'Z') b_up = 1;
    }
    for (size_t i = 0; fits && i < elen; i++) {
        if (!is_sfn_char(ext[i])) fits = 0;
        if (ext[i] >= 'a' && ext[i] <= 'z') e_lo = 1;
        if (ext[i] >= 'A' && ext[i] <= 'Z') e_up = 1;
    }

    memset(p, 0, sizeof *p);
    if (fits) {
        memset(p->sfn, ' ', 11);
        for (size_t i = 0; i < blen; i++) p->sfn[i] = (uint8_t)up(name[i]);
        for (size_t i = 0; i < elen; i++) p->sfn[8 + i] = (uint8_t)up(ext[i]);
        if (!dir_has_sfn(fs, dir_cl, p->sfn)) {
            if ((b_lo && b_up) || (e_lo && e_up)) p->lfn = 1;      // mixed case needs a long name
            else {
                if (b_lo) p->ntres |= 0x08;
                if (e_lo) p->ntres |= 0x10;
            }
            return 0;
        }
    }

    /* alias like "LONGNA~1.TXT" plus a long-name entry */
    char base[7]; int bl = 0;
    for (size_t i = 0; i < blen && bl < 6; i++)
        if (is_sfn_char(name[i])) base[bl++] = up(name[i]);
    if (bl == 0) base[bl++] = '_';
    base[bl] = 0;
    for (int n = 1; n < 100000; n++) {
        char tail[8]; int tl = 0; tail[tl++] = '~';
        char dig[8]; int dl = 0; for (int v = n; v; v /= 10) dig[dl++] = (char)('0' + v % 10);
        while (dl) tail[tl++] = dig[--dl];
        int keep = 8 - tl; if (keep > bl) keep = bl;
        memset(p->sfn, ' ', 11);
        for (int i = 0; i < keep; i++) p->sfn[i] = (uint8_t)base[i];
        for (int i = 0; i < tl; i++) p->sfn[keep + i] = (uint8_t)tail[i];
        for (size_t i = 0, k = 0; i < elen && k < 3; i++)
            if (is_sfn_char(ext[i])) p->sfn[8 + k++] = (uint8_t)up(ext[i]);
        if (!dir_has_sfn(fs, dir_cl, p->sfn)) { p->lfn = 1; p->ntres = 0; return 0; }
    }
    return -1;
}

static int dir_create_entry(fat32_t *fs, uint32_t dir_cl, const char *name, uint8_t attr,
                            uint32_t first, uint32_t size, fat32_node_t *out)
{
    name_plan_t pl;
    if (plan_name(fs, dir_cl, name, &pl) != 0) return -1;
    size_t len = strlen(name);
    int nl = pl.lfn ? (int)((len + 12) / 13) : 0;
    slot_t slots[MAX_SLOTS];
    if (dir_alloc_slots(fs, dir_cl, nl + 1, slots) != 0) return -1;

    uint8_t ck = lfn_checksum(pl.sfn);
    for (int k = 0; k < nl; k++) {                      // slots[0] holds the highest sequence number
        int ord = nl - k;
        fat32_lfn_entry_t le;
        memset(&le, 0xFF, sizeof le);
        le.ord = (uint8_t)(ord | (k == 0 ? 0x40 : 0));
        le.attr = 0x0F; le.type = 0; le.checksum = ck; le.fstClusLO = 0;
        uint16_t u[13];
        for (int i = 0; i < 13; i++) {
            size_t idx = (size_t)(ord - 1) * 13 + (size_t)i;
            u[i] = idx < len ? (uint16_t)(uint8_t)name[idx] : (idx == len ? 0x0000 : 0xFFFF);
        }
        memcpy(le.name1, u, 10); memcpy(le.name2, u + 5, 12); memcpy(le.name3, u + 11, 4);
        if (put_entry(fs, slots[k], &le) != 0) return -1;
    }

    fat32_dirent_disk_t d;
    memset(&d, 0, sizeof d);
    memcpy(d.name, pl.sfn, 11);
    d.attr = attr; d.ntres = pl.ntres;
    d.crtDate = d.wrtDate = d.lstAccDate = 0x0021;      // 1980-01-01 until a clock is available
    d.fstClusHI = (uint16_t)(first >> 16); d.fstClusLO = (uint16_t)(first & 0xFFFF);
    d.fileSize = size;
    if (put_entry(fs, slots[nl], &d) != 0) return -1;   // the 8.3 entry last: the name appears atomically

    if (out) {
        memset(out, 0, sizeof *out);
        out->fs = fs; out->first_cluster = first; out->cur_cluster = first;
        out->size = size; out->is_dir = (attr & 0x10) != 0;
        out->dirent_lba = slots[nl].lba; out->dirent_off = slots[nl].off;
        out->parent_cluster = dir_cl;
        strncpy(out->name, name, NAME_MAX); out->name[NAME_MAX - 1] = 0;
    }
    return 0;
}

static int resolve_parent(fat32_t *fs, const char *path, uint32_t *dir_cl, char *leaf)
{
    const char *slash = NULL;
    for (const char *p = path; *p; p++) if (*p == '/') slash = p;
    const char *l = slash ? slash + 1 : path;
    if (!*l || strlen(l) >= NAME_MAX) return -1;

    char parent[256];
    size_t plen = slash ? (size_t)(slash - path) : 0;
    if (plen >= sizeof parent) return -1;
    memcpy(parent, path, plen); parent[plen] = 0;

    fat32_node_t pn;
    if (lookup_path(fs, parent[0] ? parent : "/", &pn) != 0 || !pn.is_dir) return -1;
    *dir_cl = pn.first_cluster >= 2 ? pn.first_cluster : fs->root_cluster;
    strcpy(leaf, l);
    return 0;
}

/* ---- public operations ---- */
int fat32_create_file(fat32_t *fs, const char *path, fat32_node_t *out)
{

    uint32_t dir; char leaf[NAME_MAX];
    if (resolve_parent(fs, path, &dir, leaf) != 0)
    {
        log_info("FAT32", "create returning -1, create '%s' in dir cluster %u", leaf, dir);

        return -1;
    }

    log_info("FAT32", "create '%s' in dir cluster %u", leaf, dir);

    return dir_create_entry(fs, dir, leaf, 0x20, 0, 0, out);
}

int fat32_mkdir(fat32_t *fs, const char *path)
{
    uint32_t dir; char leaf[NAME_MAX];
    fat32_node_t tmp;
    if (resolve_parent(fs, path, &dir, leaf) != 0) return -1;
    if (lookup_path(fs, path, &tmp) == 0) return -1;            // already exists

    uint32_t nc = fat_alloc(fs);
    if (!nc || zero_cluster(fs, nc) != 0) return -1;
    if (dir_create_entry(fs, dir, leaf, 0x10, nc, 0, NULL) != 0) { fat_free_chain(fs, nc); return -1; }

    uint8_t sec[512];
    memset(sec, 0, fs->bytes_per_sector);
    fat32_dirent_disk_t *d = (fat32_dirent_disk_t *)sec;
    uint32_t pc = (dir == fs->root_cluster) ? 0 : dir;          // ".." of a root child points to 0
    memset(d[0].name, ' ', 11); d[0].name[0] = '.';
    memset(d[1].name, ' ', 11); d[1].name[0] = '.'; d[1].name[1] = '.';
    d[0].attr = d[1].attr = 0x10;
    d[0].fstClusHI = (uint16_t)(nc >> 16); d[0].fstClusLO = (uint16_t)(nc & 0xFFFF);
    d[1].fstClusHI = (uint16_t)(pc >> 16); d[1].fstClusLO = (uint16_t)(pc & 0xFFFF);
    d[0].crtDate = d[0].wrtDate = d[1].crtDate = d[1].wrtDate = 0x0021;
    return write_sector(fs, fat32_cluster_to_lba(fs, nc), sec);
}

int fat32_unlink(fat32_t *fs, const char *path)
{
    fat32_node_t n;
    if (lookup_path(fs, path, &n) != 0 || !n.dirent_lba) return -1;     // missing, or the root
    if (n.is_dir && !dir_is_empty(fs, n.first_cluster)) return -1;
    if (dir_delete_entry(fs, n.parent_cluster, n.dirent_lba, n.dirent_off) != 0) return -1;
    if (n.first_cluster >= 2) fat_free_chain(fs, n.first_cluster);
    return 0;
}

int fat32_rename(fat32_t *fs, const char *from, const char *to)
{
    fat32_node_t src, tmp;
    uint32_t dir; char leaf[NAME_MAX];
    if (lookup_path(fs, from, &src) != 0 || !src.dirent_lba) return -1;
    if (lookup_path(fs, to, &tmp) == 0) return -1;              // target exists (also case-only renames)
    if (resolve_parent(fs, to, &dir, leaf) != 0) return -1;
    if (src.is_dir) {                                           // never move a folder into itself
        size_t fl = strlen(from);
        if (strncmp(to, from, fl) == 0 && to[fl] == '/') return -1;
    }

    if (dir_create_entry(fs, dir, leaf, src.is_dir ? 0x10 : 0x20, src.first_cluster, src.size, NULL) != 0)
        return -1;
    if (dir_delete_entry(fs, src.parent_cluster, src.dirent_lba, src.dirent_off) != 0) return -1;

    if (src.is_dir && src.parent_cluster != dir && src.first_cluster >= 2) {   // fix ".."
        uint8_t sec[512];
        uint32_t lba = fat32_cluster_to_lba(fs, src.first_cluster);
        if (read_sector(fs, lba, sec) == 0) {
            fat32_dirent_disk_t *dd = (fat32_dirent_disk_t *)(sec + 32);
            uint32_t pc = (dir == fs->root_cluster) ? 0 : dir;
            dd->fstClusHI = (uint16_t)(pc >> 16); dd->fstClusLO = (uint16_t)(pc & 0xFFFF);
            write_sector(fs, lba, sec);
        }
    }
    return 0;
}


static int fat32_write(struct file *f, const void *buf, size_t size)
{
    fat32_node_t *n = (fat32_node_t *)f->private_data;

    log_info("FAT32", "write pos=%u size=%u file_size=%u", (unsigned)f->position, (unsigned)size, (unsigned)n->size);
    if (!n || n->is_dir || !buf) return -1;
    if (size == 0) return 0;

    fat32_t *fs = n->fs;
    uint32_t bps = fs->bytes_per_sector;
    uint32_t csz = bps * fs->sectors_per_cluster;
    if (csz == 0 || csz > MAX_CLUSTER_BYTES || bps > 4096) return -1;
    if (f->position > n->size) return -1;                       // no sparse files

    if (n->first_cluster < 2) {                                 // empty file: first cluster
        uint32_t c = fat_alloc(fs);
        if (!c) return -1;
        n->first_cluster = n->cur_cluster = c;
        n->seek_cluster = 0; n->seek_index = 0;
    }

    uint32_t target = (uint32_t)(f->position / csz);
    uint32_t skip   = (uint32_t)(f->position % csz);
    uint32_t cl = n->first_cluster, idx = 0;
    while (idx < target) {                                      // walk, extending the chain if needed
        uint32_t nx = fat_next_cluster(fs, cl);
        if (nx < 2 || is_eoc(nx)) {
            nx = fat_alloc(fs);
            if (!nx || fat_set(fs, cl, nx) != 0) return -1;
        }
        cl = nx; idx++;
    }

    const uint8_t *in = (const uint8_t *)buf;
    uint8_t bounce[512];
    size_t done = 0;

    while (done < size) {
        size_t chunk = csz - skip;
        if (chunk > size - done) chunk = size - done;
        uint32_t lba = fat32_cluster_to_lba(fs, cl);

        size_t pos = skip, left = chunk;
        const uint8_t *src = in + done;
        while (left) {
            uint32_t sec = (uint32_t)(pos / bps), off = (uint32_t)(pos % bps);
            size_t adv;
            if (off == 0 && left >= bps) {                      // whole sectors: write straight from the caller
                uint32_t ns = (uint32_t)(left / bps);
                if (write_sectors_fs(fs, lba + sec, ns, src) != 0) goto out;
                adv = (size_t)ns * bps;
            } else {                                            // partial sector: read-modify-write
                adv = bps - off; if (adv > left) adv = left;
                if (read_sector(fs, lba + sec, bounce) != 0) goto out;
                memcpy(bounce + off, src, adv);
                if (write_sector(fs, lba + sec, bounce) != 0) goto out;
            }
            pos += adv; src += adv; left -= adv; done += adv;
        }
        skip = 0;

        if (done < size) {
            uint32_t nx = fat_next_cluster(fs, cl);
            if (nx < 2 || is_eoc(nx)) {
                nx = fat_alloc(fs);
                if (!nx || fat_set(fs, cl, nx) != 0) break;
            }
            cl = nx;
        }
    }
out:
    f->position += done;
    if (f->position > n->size) n->size = (uint32_t)f->position;
    node_flush(n);
    return done ? (int)done : -1;
}