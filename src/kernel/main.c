#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "drivers/fb/fb.h"
#include "drivers/fb/fb_graphics.h"
#include "drivers/gfx/gfx.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <boot/bootparams.h>
#include "drivers/input/input.h"
#include "drivers/input/input_manager.h"
#include "compositor/compositor.h"
#include "hal/vfs_memfile.h"


#include "paging.h"
#include "pmm.h"
#include "arch/i686/gdt.h"
#include "hal/vfs.h"
#include "arch/i686/switch_user_mode.h"
#include "memfile.h"
#include "hal/partition.h"

#include <hal/elf.h>
#include <hal/process.h>
#include <hal/scheduler.h>

#include "hal/block.h"
#include "hal/fat32.h"
#include "hal/fat.h"

// #include "memdefs.h"
// #include "memory.h"

// #include "elf_loader.h"

#define PAGE_SIZE 4096

# define SEEK_SET 0
# define SEEK_CUR 1
# define SEEK_END 2

extern void _init();

void crash_me();

extern void switch_to_user_mode(uint32_t eip, uint32_t esp);


void timer(Registers* regs)
{
    printf(".");
}







// // ---------------- MEMFILE VFS BACKEND ----------------

// typedef struct {
//     uint8_t *data;
//     size_t size;
// } memfile_t;

// static int memfile_open(struct file *file) {
//     return 0;
// }

// static int memfile_read(struct file *file, void *buf, size_t size) {
//     memfile_t *mf = file->private_data;

//     if (file->position >= mf->size)
//         return 0;

//     size_t remaining = mf->size - file->position;
//     size_t tocopy = size < remaining ? size : remaining;

//     memcpy(buf, mf->data + file->position, tocopy);
//     file->position += tocopy;
//     return tocopy;
// }

// static int memfile_close(struct file *file) {
//     return 0;
// }

// // THIS MUST BE VISIBLE ABOVE YOUR CALL
// static struct file_operations memfile_fops = {
//     .open  = memfile_open,
//     .read  = memfile_read,
//     .write = NULL,
//     .close = memfile_close,
// };

















static inline void read_gdtr(uint16_t *limit, uint32_t *base)
{
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) gdtr;
    __asm__ volatile ("sgdt %0" : "=m"(gdtr));
    *limit = gdtr.limit;
    *base  = gdtr.base;
}

/* read TR (task register selector) */
static inline uint16_t read_tr(void)
{
    uint16_t tr;
    __asm__ volatile ("str %0" : "=r"(tr));
    return tr;
}

void dump_gdt_and_tr(void)
{
    uint16_t limit;
    uint32_t base;
    read_gdtr(&limit, &base);

    log_info("GDTDBG", "GDTR base=0x%08x limit=0x%04x", base, limit);

    uint32_t entries = (limit + 1) / 8;
    if (entries > 32) entries = 32; /* safety */

    for (uint32_t i = 0; i < entries; i++) {
        uint8_t *p = (uint8_t*)(base + i*8);
        uint32_t low = *(uint32_t*)&p[0];
        uint32_t high = *(uint32_t*)&p[4];
        /* Parse fields */
        uint16_t limit_low = low & 0xFFFF;
        uint16_t base_low  = (low >> 16) & 0xFFFF;
        uint8_t  base_mid  = (high & 0xFF);
        uint8_t  access    = (high >> 8) & 0xFF;
        uint8_t  flags_lim = (high >> 16) & 0xFF;
        uint8_t  base_hi   = (high >> 24) & 0xFF;
        uint32_t base_full = base_low | (base_mid << 16) | (base_hi << 24);
        uint32_t limit_full = limit_low | ((flags_lim & 0x0F) << 16);

        log_info("GDTDBG", "GDT[%02u]: base=0x%08x limit=0x%05x access=0x%02x flags=0x%02x",
                 i, base_full, limit_full, access, flags_lim);
    }

    uint16_t tr = read_tr();
    log_info("GDTDBG", "TR selector=0x%04x", tr);
}
#include <stdint.h>

static inline uint32_t read_cr3(void) {
    uint32_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

// void dump_pte(uint32_t virt_addr)
// {
//     uint32_t cr3 = read_cr3();
//     uint32_t *pd = (uint32_t*)(cr3 & 0xFFFFF000);   // physical pointer (must be identity-mapped!)
    
//     uint32_t pd_index = (virt_addr >> 22) & 0x3FF;
//     uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

//     uint32_t pde = pd[pd_index];
//     if (!(pde & 1)) {
//         log_info("dump_pte", "PDE not present\n");
//         return;
//     }

//     uint32_t *pt = (uint32_t*)(pde & 0xFFFFF000);   // physical pointer (must be identity-mapped!)

//     uint32_t pte = pt[pt_index];
//     if (!(pte & 1)) {
//         log_info("dump_pte", "PTE not present\n");
//         return;
//     }

//     uint32_t frame = pte & 0xFFFFF000;
//     log_info("dump_pte", "VA 0x%08x → frame 0x%08x\n", virt_addr, frame);
// }


// void start_userspace()
// {
//     log_info("Main", "starting userspace!");

//     log_info("Main", "starting VFS_Open /bin/init!");

//     int fd = VFS_Open("/bin/init", 0);
//     if (fd < 0) {
//         log_error("USER", "Start /bin/init open init");
//         return;
//     }
//     log_info("Main", "starting open init!");

//     // Read entire program into buffer
//     static uint8_t program[65536]; // 64KB max

//     log_info("Main", "starting open init 3!");

//     int size = VFS_Read(fd, program, sizeof(program));

//     log_info("Main", "starting VFS_Read = %d", size);

//     if (size <= 0) {
//         log_error("USER", "init read failed");
//         return;
//     }



// extern struct file_operations fs_file_ops;   // your filesystem file ops (fat/ext2/ramfs)
extern void *rootfs_super;                   // handle/context for the mounted FS

extern struct file_operations memfile_fops;

static int userspace_started = 0;

// quick_port_test.c (insert into kernel init)
void quick_port_test(void) {
    uint8_t s1 = i686_inb(0x1F7); // typical status port
    uint8_t s2 = i686_inb(0x3F6); // control
    uint8_t kb = i686_inb(0x60);  // keyboard data port (should vary)
    log_info("PORTTEST", "inb 0x1F7=0x%02x  0x3F6=0x%02x  0x60=0x%02x", s1, s2, kb);
}
// typedef struct { int i; } FILE;
// static FILE* g_file;
// static Fat g_fat;
// static const char* g_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// static bool disk_read(uint8_t* buf, uint32_t sect);
// static bool disk_write(const uint8_t* buf, uint32_t sect);

// static DiskOps g_ops =
// {
//   .read  = disk_read,
//   .write = disk_write,
// };

// //------------------------------------------------------------------------------
// static bool disk_init(const char* path)
// {
//   g_file = fopen(path, "r+");
//   return g_file != NULL;
// }

// //------------------------------------------------------------------------------
// static bool disk_read(uint8_t* buf, uint32_t sect)
// {
//   if (fseek(g_file, sect * 512, SEEK_SET))
//     return false;

//   return 1 == fread(buf, 512, 1, g_file);
// }

// //------------------------------------------------------------------------------
// static bool disk_write(const uint8_t* buf, uint32_t sect)
// {
//   if (fseek(g_file, sect * 512, SEEK_SET))
//     return false;

//   return 1 == fwrite(buf, 512, 1, g_file);
// }
// void start_userspace(void)
// {
//     log_debug("Main", "calling start_userspace");


//   int err, cnt;
//   File file;
//   Dir dir;
//   DirInfo info;
//   char buf[1024];

// //   if (argc != 2)
// //   {
// //     printf("Usage: ./demo disk.img\n");
// //     return 0;
// //   }

//   const char* path = "/";

//   if (!disk_init(path))
//     return 0;

//   // You can scan the drive for FAT32 partitions before mounting to avoid 
//   // allocating excess fat structures.
//   fat_probe(&g_ops, 0);

//   // Mount the partition under /mnt
//   fat_mount(&g_ops, 0, &g_fat, "mnt");

//   // Here are some examples:
//   printf("-------------------------------\n");
//   printf("Example 0: read large file in chunks\n");
//   {
//     err = fat_file_open(&file, "/mnt/source/fat.c", FAT_READ);
//     CHECK_ERROR(err);

//     for (;;)
//     {
//       err = fat_file_read(&file, buf, 512, &cnt);
//       CHECK_ERROR(err);

//       printf("%.*s", cnt, buf);
//       if (cnt != 512)
//         break;
//     }

//     err = fat_file_close(&file);
//     CHECK_ERROR(err);
//   }




//   fat_umount(&g_fat); // IMPORTANT
//   fclose(g_file);

// }
void init_filesystem()
{
    log_debug("Main", "calling init_filesystem");

    ata_init();

    static fat32_t fs;
    fs.bdev = block_devices[0]; // ATA disk

    log_debug("Main", "calling fat32_mount");

    fat32_mount(&fs);
}

void start_userspace(void)
{
    log_debug("Main", "calling start_userspace");

    quick_port_test();

// register_ata_devices_with_vfs(); 

    block_init();


    // ensure partition device is registered
    if (!register_first_fat32_partition()) {
        log_error("MAIN", "No FAT32 partition found during startup");
        // return or continue with diagnostics
    }


    block_device_t *part = block_lookup_by_name("sda1");
    if (!part) {
        log_error("MAIN", "Partition sda1 not found");
        return;
    }

    log_info("MAIN", "Using block device sda1: lba_base=%u sector_size=%u", part->lba_base, part->sector_size);
    

    // Initialize FAT32 fs object
    fat32_t *fs = fat32_init_device(part);

    log_debug("Main", "fat32_init_device fs = %d", fs);

    if (!fs)
    {
        log_error("MAIN", "FAT32 init failed");
        return;
    }


        // Debug — confirm BPB read correctly
    log_info("FAT32", "Mounted FAT32: bytes_per_sector=%u spc=%u reserved=%u fats=%u sectors_per_fat=%u root_cluster=%u",
             fs->bytes_per_sector, fs->sectors_per_cluster, fs->reserved_sectors,
             fs->num_fats, fs->sectors_per_fat, fs->root_cluster);

    // Mount into VFS
    VFS_Mount("/", get_fat32_fops(), fs);
    uint32_t part_lba = 0;

    log_debug("Main", "calling debug_list_root");

    debug_list_root();           // should show bin, boot, folder, etc.
    // Optional: debug_list_path("/bin");

    log_debug("Main", "calling VFS_Open");

    int fd = VFS_Open("/bin/init", O_RDONLY);
    if (fd < 0) panic("Cannot start user space");

    size_t max_size = 65536;
    memfile_t *mf = kmalloc(sizeof(memfile_t));
    mf->data = kmalloc(max_size);

    size_t total = 0;
    while (total < max_size)
    {
        int n = VFS_Read(fd, (uint8_t*)mf->data + total, 4096);

        if (n <= 0) break;
        total += (size_t)n;
    }

    mf->size = total;

    log_debug("Main", "Calling VFS_Close");
    VFS_Close(fd);
    log_debug("Main", "After VFS_Close");

    // Hand off to ELF loader
    pid_t pid = exec_elf_mem(mf->data, mf->size);
    if (pid < 0) panic("Failed to exec init");

    log_error("Main", "After exec_elf_mem");


}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    // call global constructors
    _init();

    HAL_Initialize();


    log_info("Main", "Starting pmm_init!");

    pmm_init(&bootParams->Memory);

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);

    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }

    log_info("Main", "Starting init_filesystem!");

    init_filesystem();

    // log_info("Main", "This is an info msg!");
    // log_warning("Main", "This is a warning msg!");
    // log_error("Main", "This is an error msg!");
    // log_critical("Main", "This is a critical msg!");
    // printf("Welcome to One OS v0.1\n");
    // printf("This operating system is under construction.\n");


    // initialize framebuffer
    fb_init(fb_info);
    gfx_init();            // initialize graphics layer

    log_info("MAIN", "Framebuffer ready: %ux%u", fb_dev.width, fb_dev.height);


    gfx_clear(COLOR_BLACK);              // clear screen
    log_info("Main", "After PS 2 Init!");

end:


    // compositor_init();
    // // Launch compositor
    // compositor_main();  // infinite loop

    log_info("Main", "Starting start_userspace!");
    start_userspace();


    // should never come here
    log_critical("Main", "CRITICAL ERROR: User Space not started!");
    for (;;);

}