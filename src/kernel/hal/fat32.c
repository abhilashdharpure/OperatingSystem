// fat32.c (helpers)
#include "fat32.h"
#include <string.h>
#include "debug.h"
#include "memory.h"

#define MAX_CLUSTER_BYTES 8192

#pragma pack(push, 1)
typedef struct {
    uint8_t  name[11];     // 8 chars + 3 chars
    uint8_t  attr;         // attribute bits (0x10 = directory)
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
#pragma pack(pop)

static char fat32_toupper(char c)
{
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

/* minimal snprintf: supports %s and %.*s and concat into buffer */
static int fat32_snprintf(char *dst, size_t max, const char *fmt,
                          const char *a, const char *b)
{
    // This is not a real snprintf. Just enough for NAME.EXT
    // format used: "%s"  or "%s.%s"
    size_t pos = 0;

    while (*fmt && pos < max-1)
    {
        if (*fmt == '%' && fmt[1] == 's') {
            fmt += 2;
            const char *src = a;
            if (!a) src = "";
            while (*src && pos < max-1) {
                dst[pos++] = *src++;
            }
            continue;
        }
        else if (*fmt == '%' && fmt[1] == '.' && fmt[2] == 's') {
            // Not used; ignore.
        }

        // normal char
        dst[pos++] = *fmt++;
    }

    dst[pos] = '\0';
    return (int)pos;
}

static int append_hex(char *dst, uint8_t byte, size_t max)
{
    if (max < 4) return 0;
    static const char *hex = "0123456789ABCDEF";
    dst[0] = hex[(byte >> 4) & 0xF];
    dst[1] = hex[byte & 0xF];
    dst[2] = ' ';
    dst[3] = '\0';
    return 3;
}

// static uint32_t fat32_cluster_to_lba(fat32_t *fs, uint32_t cluster)
// {
//     // cluster 2 is the first data cluster
//     // data_start_lba previously computed in fat32_parse_bpb
//     if (cluster < 2) return 0; // invalid
//     uint32_t first_data = fs->data_start_lba;
//     uint32_t lba = first_data + (cluster - 2) * fs->sectors_per_cluster;
//     return lba;
// }


static uint32_t fat32_cluster_to_lba(fat32_t *fs, uint32_t cluster)
{
    return fs->data_start_lba + (cluster - 2) * fs->sectors_per_cluster;
}

int fat32_mount(fat32_t *fs) {
    uint8_t buf[512];
    fs->bdev->read_sectors(fs->bdev, fs->bdev->lba_base, 1, buf);

    fs->bytes_per_sector   = buf[11] | (buf[12] << 8);
    fs->sectors_per_cluster= buf[13];
    fs->reserved_sectors   = buf[14] | (buf[15] << 8);
    fs->num_fats           = buf[16];
    fs->sectors_per_fat    = buf[36] | (buf[37]<<8) | (buf[38]<<16) | (buf[39]<<24);
    fs->root_cluster       = buf[44] | (buf[45]<<8) | (buf[46]<<16) | (buf[47]<<24);

    fs->fat_start_lba      = fs->bdev->lba_base + fs->reserved_sectors;
    fs->data_start_lba     = fs->fat_start_lba + fs->num_fats * fs->sectors_per_fat;

    return 0;
}

static int read_sector(fat32_t *fs, uint32_t lba, void *buf)
{
    if (!fs || !fs->bdev || !fs->bdev->read_sectors)
    {
        return -1;
    }

    // lba is absolute relative to disk start
    return fs->bdev->read_sectors(fs->bdev, fs->bdev->lba_base + lba, 1, buf);
}

// static int read_sector(fat32_t *fs, uint32_t lba, void *buf)
// {
//     if (!fs || !fs->bdev || !fs->bdev->read_sectors) {
//         log_error("FAT32", "read_sector: invalid block device fs=%p bdev=%p", fs, fs ? fs->bdev : NULL);
//         return -1;
//     }

//     // log_debug("FAT32", "read_sector: dev=%p lba_base=%u lba=%u count=1 buf=%p sec_size=%u",
//     //           fs->bdev, fs->bdev->lba_base, lba, buf, fs->bdev->sector_size);



//     return fs->bdev->read_sectors(
//         fs->bdev,
//         fs->fat_start_lba + lba,   // <-- THE FIX
//         1,
//         buf);

//     // int ok = fs->bdev->read_sectors(fs->bdev, fs->bdev->lba_base + lba, 1, buf);
//     // log_debug("FAT32", "readValueFromSector = %d", ok);

//     // uint8_t *b = (uint8_t*)buf;
//     // char tmp[64];
//     // int pos = 0;

//     // for (int i = 0; i < 16 && pos < (int)(sizeof(tmp)-4); i++)
//     //     pos += append_hex(tmp + pos, b[i], sizeof(tmp) - pos);

//     // log_debug("FAT32", "sector[0..15]=%s", tmp);
//     // return ok;
// }

// read an entire cluster into buf (buf must be large enough: bytes_per_sector * sectors_per_cluster)
static int read_cluster(fat32_t *fs, uint32_t cluster, void *buf)
{
    if (!fs || !fs->bdev || !buf) return -1;
    if (cluster < 2) return -1;

    uint32_t secs = fs->sectors_per_cluster;
    uint32_t sector = fat32_cluster_to_lba(fs, cluster);
    if (sector == 0) return -1;

    // read every sector of the cluster
    uint8_t *p = (uint8_t*)buf;
    for (uint32_t s = 0; s < secs; s++) {
        if (read_sector(fs, sector + s, p + s * fs->bytes_per_sector) != 0) {
            log_error("FAT32", "read_cluster: read_sector failed cluster=%u sector=%u", cluster, sector + s);
            return -1;
        }
    }
    return 0;
}



// static uint32_t fat_next_cluster(fat32_t *fs, uint32_t cluster) { /* ... */ }
// read next cluster from FAT (FAT32)
// static uint32_t fat_next_cluster(fat32_t *fs, uint32_t cluster)
// {
//     // log_debug("FAT32", "rfat_next_cluster start, cluster=%u", cluster);

//     if (!fs) return 0x0FFFFFFF;

//     // FAT32: 4 bytes per entry
//     uint32_t fat_offset = cluster * 4;
//     uint32_t bytes_per_sector = fs->bytes_per_sector;
//     uint32_t fat_sector = fs->fat_start_lba + (fat_offset / bytes_per_sector);
//     uint32_t offset_in_sector = fat_offset % bytes_per_sector;

//     uint8_t sector_buf[512]; // sector-sized buffer; bytes_per_sector is normally 512
//     if (bytes_per_sector > sizeof(sector_buf)) {
//         // Very unusual: support larger sectors if defined
//         // fall back to dynamic (shouldn't happen in typical systems)
//         log_error("FAT32", "fat_next_cluster: unsupported bytes_per_sector=%u", bytes_per_sector);
//         return 0x0FFFFFFF;
//     }

//     if (read_sector(fs, fat_sector, sector_buf) != 0) {
//         log_error("FAT32", "fat_next_cluster: read_sector failed fat_sector=%u", fat_sector);
//         return 0x0FFFFFFF;
//     }

//     // read 4 bytes little-endian
//     uint32_t entry = *(uint32_t*)(sector_buf + offset_in_sector);
//     entry &= 0x0FFFFFFF; // mask top nybble (FAT32 uses 28 bits)
//     return entry;
// }

static uint32_t fat_next_cluster(fat32_t *fs, uint32_t cluster)
{
    if (!fs) return 0x0FFFFFFF;

    uint32_t fat_offset = cluster * 4;
    uint32_t bytes_per_sector = fs->bytes_per_sector;
    uint32_t fat_sector = fs->fat_start_lba + (fat_offset / bytes_per_sector);
    uint32_t offset_in_sector = fat_offset % bytes_per_sector;

    // Make sure buffer is in high-half kernel
    uint8_t clbuf[512] __attribute__((aligned(4)));

    if (read_sector(fs, fat_sector, clbuf) != 0) {
        log_error("FAT32", "fat_next_cluster: read_sector failed fat_sector=%u", fat_sector);
        return 0x0FFFFFFF;
    }

    uint32_t entry;
    memcpy(&entry, clbuf + offset_in_sector, sizeof(entry)); // avoids misaligned access
    entry &= 0x0FFFFFFF; // mask top 4 bits
    return entry;
}



// static int is_eoc(uint32_t cl)
// {
//     // log_debug("FAT32", "is_eoc c = %d",cl);
//     return cl >= 0x0FFFFFF8 && cl <= 0x0FFFFFFF;
//     // return cl >= 0x0FFFFFF8;
// }

static int is_eoc(uint32_t cl)
{
    cl &= 0x0FFFFFFF;  // mask upper 4 bits
    return cl >= 0x0FFFFFF8;
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
        name[i] = fat32_toupper(name[i]);
    for (int i = 0; ext[i]; i++)
        ext[i] = fat32_toupper(ext[i]);

    if (ext[0]) {
        // NAME.EXT version
        size_t pos = 0;
        size_t max = NAME_MAX;

        // copy name
        for (size_t i = 0; name[i] && pos < max-1; i++)
            out[pos++] = name[i];

        if (pos < max-1) out[pos++] = '.';

        for (size_t i = 0; ext[i] && pos < max-1; i++)
            out[pos++] = ext[i];

        out[pos] = '\0';
    }
    else {
        // NAME only
        size_t pos = 0;
        size_t max = NAME_MAX;

        for (size_t i = 0; name[i] && pos < max-1; i++)
            out[pos++] = name[i];

        out[pos] = '\0';
    }
}

// static int split_next(const char **p, char out[NAME_MAX]) { /* ... */ }
// split next path component from *p_in; writes component into out (NAME_MAX). 
// Advances *p_in to character after '/' (or to '\0'). Returns 1 if a component was produced, 0 if no more components.
static int split_next(const char **p_in, char out[NAME_MAX])
{
    if (!p_in || !*p_in || !out) return 0;
    const char *p = *p_in;

    // skip initial slashes
    while (*p == '/') p++;
    if (*p == '\0') {
        *p_in = p;
        return 0;
    }

    int oi = 0;
    while (*p && *p != '/' && oi < NAME_MAX - 1) {
        out[oi++] = *p++;
    }
    out[oi] = '\0';

    // skip following slash(es)
    while (*p == '/') p++;
    *p_in = p;
    return 1;
}

// Case-insensitive 8.3 name builder
static void fat32_make_83(const char *input, uint8_t out[11])
{
    memset(out, ' ', 11);

    int i = 0;
    int pos = 0;

    // name part (max 8 chars)
    while (input[i] && input[i] != '.' && pos < 8) {
        char c = input[i++];
        if (c >= 'a' && c <= 'z')
            c -= 32;
        out[pos++] = c;
    }

    // extension part
    if (input[i] == '.') {
        i++;
        pos = 8;
        int ext_i = 0;

        while (input[i] && ext_i < 3) {
            char c = input[i++];
            if (c >= 'a' && c <= 'z')
                c -= 32;
            out[pos++] = c;
            ext_i++;
        }
    }
}

static void simple_strcpy(char *dst, const char *src, size_t max)
{
    size_t i = 0;
    while (i + 1 < max && src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}



// ------------------------------------------------------------
//                PURE C IMPLEMENTATION: lookup_path
// ------------------------------------------------------------
int lookup_path(fat32_t *fs, const char *subpath, fat32_node_t *out)
{
    // log_debug("FAT32", "lookup_path, subpath = %s", subpath);
    if (!fs || !out)
    {
        log_debug("FAT32", "lookup_path 1");
        return -1;
    }

    // skip leading '/'
    const char *p = subpath;
    if (*p == '/')
    {
        p++;
    }

    // empty path → root
    if (*p == '\0')
    {
        memset(out, 0, sizeof(*out));
        out->fs = fs;
        out->first_cluster = fs->root_cluster;
        out->is_dir = 1;
        strncpy(out->name, "/", NAME_MAX);
        log_debug("FAT32", "lookup_path subpath is empty, returning");
        return 0;
    }

    // buffer for cluster
    uint32_t cluster_bytes = fs->bytes_per_sector * fs->sectors_per_cluster;

    if (cluster_bytes > MAX_CLUSTER_BYTES)
    {

        log_debug("FAT32", "lookup_path 3");
        return -1;
    }

    uint8_t cluster_buf[MAX_CLUSTER_BYTES];

    uint32_t current_cluster = fs->root_cluster;

    char component[NAME_MAX];
    // log_debug("FAT32", "lookup_path P = %d",p);
    while (*p) {
        // Extract 1 path component
        int ci = 0;
        while (*p && *p != '/' && ci < NAME_MAX-1)
        {
            component[ci] = *p++;
            // log_debug("FAT32", "lookup_path component[ci] = %c",component[ci]);
            ci++;
        }

        // log_debug("FAT32", "lookup_path ci = %d",ci);

        component[ci] = '\0';
        if (*p == '/') p++;

        if (ci == 0)
        {
            // log_debug("FAT32", "lookup_path Askip as ci == 0 ");
            continue; // skip empty //
        }

        // Create 8.3 form
        uint8_t want83[11];
        fat32_make_83(component, want83);

        int found = 0;
        uint32_t cl = current_cluster;

        // log_debug("FAT32", "lookup_path Before While loop cl = %d", cl);


        // Search clusters of directory
        while (!is_eoc(cl))
        {
            // log_debug("FAT32", "lookup_path While loop cl = %d", cl);

            uint32_t first_sector = fat32_cluster_to_lba(fs, cl);

            // log_debug("FAT32", "lookup_path first_sector = %d", first_sector);

            for (uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
                uint32_t lba = first_sector + s;

                if (read_sector(fs, lba, cluster_buf) != 0)
                    return -1;

                // FAT entries (32 bytes each)
                uint32_t entries = cluster_bytes / 32;
                for (uint32_t e = 0; e < entries; e++) {
                    fat32_dirent_disk_t *de =
                        (fat32_dirent_disk_t*)(cluster_buf + e*32);

                    // end of directory
                    if (de->name[0] == 0x00)
                        goto search_fail;

                    // deleted or LFN
                    if (de->name[0] == 0xE5) continue;
                    if ((de->attr & 0x0F) == 0x0F) continue;

                    // compare 11-byte short name
                    if (memcmp(de->name, want83, 11) == 0) {
                        // Found
                        uint32_t first =
                            ((uint32_t)de->fstClusHI << 16) |
                             (uint32_t)de->fstClusLO;

                        memset(out, 0, sizeof(*out));
                        out->fs = fs;
                        out->first_cluster = first;
                        out->size = de->fileSize;
                        out->is_dir = (de->attr & 0x10) != 0;
                        out->cur_cluster = first;
                        // snprintf(out->name, NAME_MAX, "%s", component);
                        simple_strcpy(out->name, component, NAME_MAX);


                        found = 1;
                        goto search_done;
                    }
                }
            }

search_fail:
            // next cluster
            cl = fat_next_cluster(fs, cl);
        }

search_done:
        if (!found)
        {
            return -1;
        }

        // If last component → return result
        if (*p == '\0')
        {
            return 0;
        }

        // Must be a directory if continuing deeper
        if (!out->is_dir)
        {
            return -1;
        }

        current_cluster = out->first_cluster;
    }

    // log_debug("FAT32", "lookup_path end");
    return -1;
}



#pragma pack(push, 1)
typedef struct {
    uint8_t boot_flag;      // 0x80 bootable
    uint8_t chs_start[3];
    uint8_t type;           // 0x0B/0x0C for FAT32
    uint8_t chs_end[3];
    uint32_t lba_start;     // little-endian
    uint32_t sectors_total; // little-endian
} mbr_part_t;
#pragma pack(pop)

// typedef struct {
//     uint8_t boot_flag;      // 0x80 bootable
//     uint8_t chs_start[3];
//     uint8_t type;           // 0x0B/0x0C for FAT32 (CHS/LBA)
//     uint8_t chs_end[3];
//     uint32_t lba_start;     // little-endian
//     uint32_t sectors_total; // little-endian
// } __attribute__((packed)) mbr_part_t;

// static int fat32_parse_bpb(fat32_t *fs)
// {
//     uint8_t sec[512];
//     if (read_sector(fs, 0, sec) != 0) {  // lba=0 means fs->bdev->lba_base + 0
//         log_error("FAT32", "BPB read failed");
//         return -1;
//     }

//     // read_sector(fs, 0, sec);  

//     log_debug("FAT32", "After read_sector");

//     // Signature at 0x1FE
//     uint16_t sig = *(uint16_t*)&sec[510];
//     log_debug("FAT32", "After read_sector, sig = %d", sig);

//     // if (sig != 0xAA55)
//     // {
//     //     log_error("FAT32", "Invalid VBR signature: 0x%04x (expected 0xAA55)", sig);
//     //     return -1;
//     // }



//     // Parse core BPB
//     uint16_t bytes_per_sector     = *(uint16_t*)&sec[11];
//     uint8_t  sectors_per_cluster  = sec[13];
//     uint16_t reserved_sectors     = *(uint16_t*)&sec[14];
//     uint8_t  num_fats             = sec[16];
//     uint16_t root_entry_count     = *(uint16_t*)&sec[17];
//     uint16_t total_sectors16      = *(uint16_t*)&sec[19];
//     uint16_t sectors_per_fat16    = *(uint16_t*)&sec[22];
//     uint32_t total_sectors32      = *(uint32_t*)&sec[32];
//     uint32_t sectors_per_fat32    = *(uint32_t*)&sec[36];
//     uint32_t root_cluster         = *(uint32_t*)&sec[44];

//     uint32_t total_sectors = total_sectors32 ? total_sectors32 : total_sectors16;

//     log_error("FAT32", "Read sectors_per_fat32=%u", sectors_per_fat32);
//     log_error("FAT32", "Read sectors_per_fat16=%u", sectors_per_fat16);

//     uint32_t sectors_per_fat = sectors_per_fat32 ? sectors_per_fat32 : sectors_per_fat16;

//     log_debug("FAT32", "BPB: bps=%u spc=%u rsv=%u fats=%u rootEnt=%u tot=%u spf=%u rootClus=%u",
//               bytes_per_sector, sectors_per_cluster, reserved_sectors, num_fats,
//               root_entry_count, total_sectors, sectors_per_fat, root_cluster);

//     // Sanity checks
//     if (bytes_per_sector != 512 && bytes_per_sector != 1024 &&
//         bytes_per_sector != 2048 && bytes_per_sector != 4096) {
//         log_error("FAT32", "Unsupported bytes_per_sector=%u", bytes_per_sector);
//         // return -1;
//     }
//     if (sectors_per_cluster == 0 || (sectors_per_cluster & (sectors_per_cluster - 1)) != 0) {
//         log_error("FAT32", "Invalid sectors_per_cluster=%u", sectors_per_cluster);
//         // return -1;
//     }
//     if (num_fats < 1 || num_fats > 2) {
//         log_error("FAT32", "Unexpected num_fats=%u", num_fats);
//         // return -1;
//     }
//     if (sectors_per_fat == 0) {
//         log_error("FAT32", "sectors_per_fat is zero");
//         return -1;
//     }
//     if (root_cluster < 2) {
//         log_error("FAT32", "root_cluster=%u invalid", root_cluster);
//         return -1;
//     }
//     // FAT32 should have root_entry_count == 0; if not, still allow (hybrid images exist)
//     if (root_entry_count != 0) {
//         log_debug("FAT32", "Note: root_entry_count=%u (FAT12/16 style) but continuing", root_entry_count);
//     }

//     // Compute layout
//     uint32_t fat_start_lba  = reserved_sectors;
//     uint32_t data_start_lba = reserved_sectors + num_fats * sectors_per_fat;

//     fs->bytes_per_sector    = bytes_per_sector;
//     fs->sectors_per_cluster = sectors_per_cluster;
//     fs->reserved_sectors    = reserved_sectors;
//     fs->num_fats            = num_fats;
//     fs->sectors_per_fat     = sectors_per_fat;
//     fs->root_cluster        = root_cluster;
//     fs->fat_start_lba       = fat_start_lba;
//     fs->data_start_lba      = data_start_lba;

//     log_debug("FAT32", "Layout: fat_start=%u data_start=%u", fs->fat_start_lba, fs->data_start_lba);
//     return 0;
// }


// -----------------------------
// Parse BPB (VBR) at given LBA
// -----------------------------
static int fat32_parse_bpb(fat32_t *fs, uint32_t vbr_lba)
{
    uint8_t sec[512];
    if (read_sector(fs, vbr_lba, sec) != 0) {
        log_error("FAT32", "BPB read failed");
        return -1;
    }

    // Check VBR signature
    if (sec[510] != 0x55 || sec[511] != 0xAA) {
        log_error("FAT32", "Invalid VBR signature");
        return -1;
    }

    // Parse BPB fields
    uint16_t bytes_per_sector     = *(uint16_t*)&sec[11];
    uint8_t  sectors_per_cluster  = sec[13];
    uint16_t reserved_sectors     = *(uint16_t*)&sec[14];
    uint8_t  num_fats             = sec[16];
    uint16_t root_entry_count     = *(uint16_t*)&sec[17];
    uint16_t total_sectors16      = *(uint16_t*)&sec[19];
    uint16_t sectors_per_fat16    = *(uint16_t*)&sec[22];
    uint32_t total_sectors32      = *(uint32_t*)&sec[32];
    uint32_t sectors_per_fat32    = *(uint32_t*)&sec[36];
    uint32_t root_cluster         = *(uint32_t*)&sec[44];

    uint32_t total_sectors = total_sectors32 ? total_sectors32 : total_sectors16;
    uint32_t sectors_per_fat = sectors_per_fat32 ? sectors_per_fat32 : sectors_per_fat16;

    log_info("FAT32", "BPB: bytes/sec=%u spc=%u rsv=%u fats=%u root=%u tot=%u spf=%u rootClus=%u",
             bytes_per_sector, sectors_per_cluster, reserved_sectors,
             num_fats, root_entry_count, total_sectors, sectors_per_fat, root_cluster);

    // Sanity checks
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

    // Compute layout
    fs->bytes_per_sector    = bytes_per_sector;
    fs->sectors_per_cluster = sectors_per_cluster;
    fs->reserved_sectors    = reserved_sectors;
    fs->num_fats            = num_fats;
    fs->sectors_per_fat     = sectors_per_fat;
    fs->root_cluster        = root_cluster;
    fs->fat_start_lba       = reserved_sectors;
    fs->data_start_lba      = reserved_sectors + num_fats * sectors_per_fat;

    log_info("FAT32", "Layout: fat_start=%u data_start=%u", fs->fat_start_lba, fs->data_start_lba);

    return 0;
}

// Adapt your file_operations to use subpath and private_data
static int fat32_open(struct file *f, int flags)
{
    // log_info("FAT32", "fat32_open");

    fat32_t *fs = (fat32_t*)f->private_data;
    fat32_node_t *node = (fat32_node_t*)kmalloc(sizeof(fat32_node_t));
    if (!node) return -1;

    // log_info("FAT32", "fat32_open f->subpath = %s" , f->subpath);
    // log_info("FAT32", "fat32_open f->subpath[0] = %c" , f->subpath[0]);

    if (f->subpath && f->subpath[0] != '\0')
    {
        // log_info("FAT32", "fat32_open Not Dir");

        if (lookup_path(fs, f->subpath, node) != 0)
        { 
            log_info("FAT32", "lookup_path returns not 0");
            kfree(node);
            return -1;
        }
    }
    else
    {
        // log_info("FAT32", "fat32_open Dir");
        // open root directory
        node->fs = fs;
        node->first_cluster = fs->root_cluster;
        node->size = 0;
        node->is_dir = 1;
        strcpy(node->name, "/");
    }
    node->cur_cluster = node->first_cluster;
    node->cur_offset = 0;
    f->private_data = node; // replace ctx with node for this open handle
    return 0;
}

static int fat32_close(struct file *f) {
    if (f && f->private_data) { kfree(f->private_data); f->private_data = NULL; }
    return 0;
}

static int fat32_readdir(struct file *f, dirent_t *e)
{
    fat32_node_t *node = (fat32_node_t*)f->private_data;
    if (!node || !node->is_dir) return -1;

    uint32_t cur = node->cur_cluster;
    uint8_t cluster_buf[4096];

    while (!is_eoc(cur)) {
        if (read_cluster(node->fs, cur, cluster_buf) != 0) return -1;

        size_t cluster_bytes = node->fs->sectors_per_cluster * node->fs->bytes_per_sector;
        while (node->cur_offset < cluster_bytes) {
            fat32_dirent_disk_t *d = (fat32_dirent_disk_t*)(cluster_buf + node->cur_offset);
            node->cur_offset += sizeof(fat32_dirent_disk_t);

            if (d->name[0] == 0x00) return -1;       // end
            if (d->name[0] == 0xE5) continue;        // deleted
            if ((d->attr & 0x0F) == 0x0F) continue;  // LFN

            make_short_name(d, e->name);
            e->inode = 0;
            e->type = (d->attr & 0x10) ? 1 : 0;
            return 0; // one entry
        }
        // move to next cluster in chain
        uint32_t next = fat_next_cluster(node->fs, cur);
        node->cur_cluster = cur = next;
        node->cur_offset = 0;
    }
    return -1;
}

static int fat32_read(struct file *f, void *buf, size_t size)
{
    // log_info("FAT32", "fat32_read");
    fat32_node_t *node = (fat32_node_t*)f->private_data;

    // log_info("FAT32", "fat32_read node = %d", node);
    // log_info("FAT32", "fat32_read node->is_dir = %d", node->is_dir);

    if (!node || node->is_dir)
    {
        log_info("FAT32", "fat32_read if (!node || node->is_dir)");

        return -1;
    }

    size_t remaining = size;
    size_t read_total = 0;
    size_t file_pos = f->position;

    uint32_t cluster_size = node->fs->bytes_per_sector * node->fs->sectors_per_cluster;
    uint32_t cluster = node->first_cluster;

    // Skip clusters until we reach position
    size_t skip = file_pos;
    while (skip >= cluster_size && !is_eoc(cluster)) {
        cluster = fat_next_cluster(node->fs, cluster);
        skip -= cluster_size;
    }
    if (is_eoc(cluster))
    {
        log_info("FAT32", "fat32_read if (is_eoc(cluster))");

        return 0;
    }

    uint8_t clbuf[4096];
    while (remaining > 0 && !is_eoc(cluster)) {
        if (read_cluster(node->fs, cluster, clbuf) != 0) break;
        size_t chunk = cluster_size - skip;
        if (chunk > remaining) chunk = remaining;

        memcpy((uint8_t*)buf + read_total, clbuf + skip, chunk);
        read_total += chunk;
        remaining -= chunk;
        skip = 0; // only skip first cluster

        // log_info("FAT32", "fat32_read calling fat_next_cluster");

        cluster = fat_next_cluster(node->fs, cluster);
    }

    f->position += read_total;
    if (f->position > node->size)
    {
        f->position = node->size;
    }

    // log_info("FAT32", "fat32_read read_total = %d", read_total);

    return (int)read_total;
}

// dev->read_sectors must read 1 sector into buf
// returns 0 on success and sets *out_lba_start, otherwise -1
int find_fat32_partition(block_device_t *dev, uint32_t *out_lba_start)
{
    uint8_t sec[512];

    if (!dev || !dev->read_sectors) return -1;

    if (dev->read_sectors(dev, /*lba=*/0, /*count=*/1, sec) != 0) {
        log_error("PART", "Failed to read LBA 0");
        return -1;
    }

    // print the signature bytes for debugging
    log_info("PART", "MBR sig bytes: %02x %02x", sec[510], sec[511]);

    // check MBR signature
    if (sec[510] != 0x55 || sec[511] != 0xAA) {
        log_info("PART", "MBR signature invalid: 0x%02x 0x%02x", sec[510], sec[511]);
        return -1; // no proper MBR -> fallback to raw
    }

    // parse 4 partition entries at 0x1BE
    for (int i = 0; i < 4; i++) {
        mbr_part_t *p = (mbr_part_t*)(sec + 0x1BE + i * 16);
        log_debug("PART", "part[%d] type=0x%02x lba_start=%u sectors=%u", i, p->type, p->lba_start, p->sectors_total);
        if (p->type == 0x0B || p->type == 0x0C) {
            // found FAT32 partition
            *out_lba_start = p->lba_start;
            log_info("PART", "Found FAT32 partition at LBA %u (entry %d)", p->lba_start, i);
            return 0;
        }
    }

    log_info("PART", "No FAT32 partition in MBR");
    return -1;
}

// helper: parse MBR and register first partition as e.g. "sda1"
int register_mbr_partitions(block_device_t *disk)
{
    if (!disk || !disk->read_sectors) return -1;

    uint8_t sec[512];
    if (disk->read_sectors(disk, disk->lba_base + 0, 1, sec) != 0) {
        log_error("PART", "Failed to read MBR (LBA 0)");
        return -1;
    }

    log_error("PART", "MBR signature invalid: sec[510] = %d", sec[510]);
    log_error("PART", "MBR signature invalid: sec[511] = %d", sec[511]);


    // verify signature
    if (sec[510] != 0x55 || sec[511] != 0xAA) {
        log_error("PART", "MBR signature invalid: 0x%02x 0x%02x", sec[510], sec[511]);
        return -1;
    }

    // partition table entry offset
    const size_t PTE_OFFSET = 0x1BE;
    mbr_part_t *p = (mbr_part_t*)&sec[PTE_OFFSET];

    // Only handle first entry here (extend as needed)
    if (p->type == 0x00) {
        log_info("PART", "No partition in entry 0");
        return -1;
    }

    uint32_t part_lba_start = p->lba_start;
    uint32_t part_sectors = p->sectors_total;

    log_info("PART", "Found partition 1: type=0x%02x lba_start=%u sectors=%u", p->type, part_lba_start, part_sectors);

    // create a new block_device for partition (caller must have a table to register it)
    // Allocate / populate a new block_device_t. Adjust allocation method for your kernel.
    block_device_t *partdev = (block_device_t*)kmalloc(sizeof(block_device_t));
    if (!partdev) {
        log_error("PART", "Out of memory creating partition device");
        return -1;
    }

    // Fill fields: read_sectors should point to same backend (ata_read_sectors),
    // lba_base must be disk->lba_base + part_lba_start
    partdev->read_sectors = disk->read_sectors;
    partdev->lba_base     = disk->lba_base + part_lba_start;
    partdev->sector_size  = disk->sector_size;
    partdev->private_data = disk->private_data; // same ata context

    // register under name e.g. "sda1"
    // You likely have a registry function in your block layer, example:
    if (register_block_device("sda1", partdev) != 0) {
        log_error("PART", "Failed to register sda1");
        kfree(partdev);
        return -1;
    }

    log_info("PART", "Registered partition sda1 with lba_base=%u", partdev->lba_base);
    return 0;
}


// fat32_t* fat32_init_device(block_device_t *bdev)
// {
//     if (!bdev) {
//         log_error("FAT32", "fat32_init_device: bdev=NULL");
//         return NULL;
//     }

//     fat32_t *fs = (fat32_t*)kmalloc(sizeof(fat32_t));
//     if (!fs) {
//         log_error("FAT32", "fat32_init_device: fs=NULL");
//         return NULL;
//     }
//     memset(fs, 0, sizeof(*fs));
//     fs->bdev = bdev;

//     // -----------------------------
//     // Read MBR (LBA 0)
//     // -----------------------------
//     uint8_t mbr[512];
//     if (read_sector(fs, 0, mbr) != 0) {
//         log_error("FAT32", "Failed to read MBR");
//         kfree(fs);
//         return NULL;
//     }

//     // Check MBR signature
//     if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
//         log_error("FAT32", "Invalid MBR signature");
//         kfree(fs);
//         return NULL;
//     }

//     // Find first FAT32 partition (type 0x0B or 0x0C)
//     mbr_part_t *part = (mbr_part_t*)&mbr[0x1BE];
//     int found = 0;
//     for (int i = 0; i < 4; i++, part++) {
//         if (part->type == 0x0B || part->type == 0x0C) {
//             found = 1;
//             // fs->bdev->lba_base = part->lba_start; // <-- crucial: VBR offset
//             log_info("FAT32", "Found FAT32 partition: start LBA = %u", part->lba_start);
//             break;
//         }
//     }

//     if (!found) {
//         log_error("FAT32", "No FAT32 partition found");
//         kfree(fs);
//         return NULL;
//     }

//     // Parse VBR at partition start
//     if (fat32_parse_bpb(fs, fs->bdev->lba_base) != 0) {
//         log_error("FAT32", "Failed to parse FAT32 VBR");
//         kfree(fs);
//         return NULL;
//     }

//     return fs;
// }

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

    // If caller passed a partition device (lba_base != 0), do NOT attempt to
    // parse the disk MBR here — parse the VBR at partition start instead.
    if (bdev->lba_base != 0) {
        log_info("FAT32", "fat32_init_device: device appears to be a partition (lba_base=%u) — parsing VBR at partition start", bdev->lba_base);
        if (fat32_parse_bpb(fs, /*vbr_lba=*/ 0) != 0) { // read_sector() will add bdev->lba_base
            log_error("FAT32", "Failed to parse VBR on partition device");
            kfree(fs);
            return NULL;
        }
        return fs;
    }

    // Otherwise, caller passed a raw disk device — read MBR and locate partition.
    uint8_t mbr[512];
    if (read_sector(fs, /*lba=*/0, mbr) != 0) {
        log_error("FAT32", "Failed to read MBR");
        kfree(fs);
        return NULL;
    }

    // Check MBR signature
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        log_error("FAT32", "Invalid MBR signature");
        kfree(fs);
        return NULL;
    }

    // Find first FAT32 partition (type 0x0B or 0x0C)
    mbr_part_t *part = (mbr_part_t*)&mbr[0x1BE];
    int found = 0;
    for (int i = 0; i < 4; i++, part++) {
        if (part->type == 0x0B || part->type == 0x0C) {
            found = 1;
            // do NOT mutate bdev->lba_base here (caller owns device object)
            // instead call parse_bpb with absolute VBR LBA (part->lba_start)
            log_info("FAT32", "Found FAT32 partition in MBR: start LBA = %u (entry %d)", part->lba_start, i);

            if (fat32_parse_bpb(fs, part->lba_start) != 0) {
                log_error("FAT32", "Failed to parse VBR at LBA %u", part->lba_start);
                kfree(fs);
                return NULL;
            }
            break;
        }
    }

    if (!found) {
        log_error("FAT32", "No FAT32 partition found in MBR");
        kfree(fs);
        return NULL;
    }

    return fs;
}




// fat32_t* fat32_init_device(block_device_t *bdev)
// {
//     if (!bdev) {
//         log_error("FAT32", "fat32_init_device: bdev=NULL");
//         return NULL;
//     }

//     fat32_t *fs = (fat32_t*)kmalloc(sizeof(fat32_t));
//     if (!fs)
//     {
//         log_error("FAT32", "fat32_init_device: fs=NULL");
//         return NULL;
//     }

//     log_error("FAT32", "fat32_init_device: fs = %d ",fs);

//     memset(fs, 0, sizeof(*fs));
//     fs->bdev = bdev;

//     log_error("FAT32", "fat32_init_device: after fs = %d ",fs);


//     // if (fat32_parse_bpb(fs) != 0)
//     // {
//     //     log_error("FAT32", "fat32_init_device: fat32_parse_bpb!=0");
//     //     kfree(fs);
//     //     return NULL;
//     // }

//     uint8_t mbr[512];
//     if (fs->bdev->read_sectors(fs->bdev, 0, 1, mbr) != 0) {
//         log_error("FAT32", "Failed to read MBR");
//         return -1;
//     }

//     mbr_part_t *p = (mbr_part_t*)&mbr[446];

//     if (p->type != 0x0B && p->type != 0x0C) {
//         log_error("FAT32", "No FAT32 partition found");
//         return -1;
//     }

//     fs->fat_start_lba = p->lba_start;

//     log_debug("FAT32", "FAT32 partition starts at LBA %u", fs->fat_start_lba);



//     return fs;
// }

static struct file_operations fat32_fops = {
    .open    = fat32_open,
    .close   = fat32_close,
    .read    = fat32_read,
    .write   = NULL,
    .ioctl   = NULL,
    .readdir = fat32_readdir,
};


struct file_operations *get_fat32_fops(void) { return &fat32_fops; }