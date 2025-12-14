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

void timer(Registers* regs)
{
    printf(".");
}

// extern struct file_operations fs_file_ops;   // your filesystem file ops (fat/ext2/ramfs)
extern void *rootfs_super;                   // handle/context for the mounted FS

extern struct file_operations memfile_fops;

static int userspace_started = 0;

// // quick_port_test.c (insert into kernel init)
// void quick_port_test(void) {
//     uint8_t s1 = i686_inb(0x1F7); // typical status port
//     uint8_t s2 = i686_inb(0x3F6); // control
//     uint8_t kb = i686_inb(0x60);  // keyboard data port (should vary)
//     log_info("PORTTEST", "inb 0x1F7=0x%02x  0x3F6=0x%02x  0x60=0x%02x", s1, s2, kb);
// }

void init_filesystem()
{
    // log_debug("Main", "calling init_filesystem");

    ata_init();

    static fat32_t fs;
    fs.bdev = block_devices[0]; // ATA disk

    // log_debug("Main", "calling fat32_mount");

    fat32_mount(&fs);
}

void start_userspace(BootParams* bootParams)
{
    // log_debug("Main", "calling start_userspace");

    // quick_port_test();

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

    // log_info("MAIN", "Using block device sda1: lba_base=%u sector_size=%u", part->lba_base, part->sector_size);
    

    // Initialize FAT32 fs object
    fat32_t *fs = fat32_init_device(part);

    // log_debug("Main", "fat32_init_device fs = %d", fs);

    if (!fs)
    {
        log_error("MAIN", "FAT32 init failed");
        return;
    }

        // Debug — confirm BPB read correctly
    // log_info("FAT32", "Mounted FAT32: bytes_per_sector=%u spc=%u reserved=%u fats=%u sectors_per_fat=%u root_cluster=%u",
    //          fs->bytes_per_sector, fs->sectors_per_cluster, fs->reserved_sectors,
    //          fs->num_fats, fs->sectors_per_fat, fs->root_cluster);

    // Mount into VFS
    VFS_Mount("/", get_fat32_fops(), fs);
    uint32_t part_lba = 0;

    debug_list_root();           // should show bin, boot, folder, etc.
    // Optional: debug_list_path("/bin");

    // log_debug("Main", "calling VFS_Open");

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

    // log_debug("Main", "Calling VFS_Close");
    VFS_Close(fd);
    // log_debug("Main", "After VFS_Close");

    // Hand off to ELF loader
    pid_t pid = exec_elf_mem(mf->data, mf->size, bootParams);
    if (pid < 0)
    {
        panic("Failed to exec init");
    }

}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    // log_info("Main", "Kernel Main started...");

    // call global constructors
    _init();
    // log_info("Main", "Kernel staring HAL init...");

    HAL_Initialize();


    // log_info("Main", "Starting pmm_init!");

    pmm_init(&bootParams->Memory);

    set_kernel_page_directory();

    // log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    // log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);

    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_info("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }

    // log_info("Main", "Starting init_filesystem!");

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

    // log_info("MAIN", "Framebuffer ready: %ux%u", fb_dev.width, fb_dev.height);


    gfx_clear(COLOR_BLACK);              // clear screen

    set_phys_to_virt_ready();
end:


    // compositor_init();
    // // Launch compositor
    // compositor_main();  // infinite loop

    log_info("Main", "Starting start_userspace!");
    start_userspace(bootParams);


    // should never come here
    log_critical("Main", "CRITICAL ERROR: User Space not started!");

    for (;;);
}