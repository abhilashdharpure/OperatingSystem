#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "drivers/fb/fb.h"
#include "drivers/fb/fb_graphics.h"
#include "drivers/gfx/gfx.h"
#include <hal/hal.h>
#include <arch/x86_64/irq.h>
#include <arch/x86_64/cpu.h>
#include <debug.h>
#include <boot/bootparams.h>
#include "drivers/input/input.h"
#include "drivers/input/input_manager.h"
#include "compositor/compositor.h"
#include "hal/vfs_memfile.h"
#include <arch/x86_64/io.h>

#include "multiboot2_defines.h"
#include "arch/x86_64/serial.h"

#include "paging.h"
#include "pmm.h"
#include "arch/x86_64/gdt.h"
#include "hal/vfs.h"
#include "memfile.h"
#include "hal/partition.h"

#include <hal/elf.h>
#include <hal/process.h>
#include <hal/scheduler.h>

#include "hal/block.h"
#include "hal/fat32.h"
#include "hal/fat.h"

// #define PAGE_SIZE 4096

extern uint8_t _kernel_stack_top;

#define VGA_PHYS 0xB8000


// extern void _init();

void crash_me();


extern uint32_t mb_info_ptr;
void start(BootParams* bootParams, VbeModeInfo* fb_info);

void timer(ISRFrame64* regs)
{
    printf(".");
}

void init_filesystem()
{
    ata_init();

    static fat32_t fs;
    fs.bdev = block_devices[0]; // ATA disk

    fat32_mount(&fs);
}

void start_userspace(BootParams* bootParams)
{
    // log_debug("Main", "calling start_userspace");

    block_init();

    // ensure partition device is registered
    if (!register_first_fat32_partition())
    {
        log_error("MAIN", "No FAT32 partition found during startup");
        // return or continue with diagnostics
    }

    block_device_t *part = block_lookup_by_name("sda1");
    if (!part)
    {
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

    // log_info("FAT32", "Mounted FAT32: bytes_per_sector=%u spc=%u reserved=%u fats=%u sectors_per_fat=%u root_cluster=%u",
    //          fs->bytes_per_sector, fs->sectors_per_cluster, fs->reserved_sectors,
    //          fs->num_fats, fs->sectors_per_fat, fs->root_cluster);

    // Mount into VFS
    VFS_Mount("/", get_fat32_fops(), fs);
    uint32_t part_lba = 0;

    debug_list_root();           // should show bin, boot, folder, etc.

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
    VFS_Close(fd);

    // Hand off to ELF loader
    pid_t pid = exec_elf_mem(mf->data, mf->size, bootParams);
    if (pid < 0)
    {
        panic("Failed to exec init");
    }
}


void parse_multiboot2(void *mb_info_ptr) 
{ 
    (void)mb_info_ptr; // TODO: implement real Multiboot2 parsing for x86_64 
    
}

void parse_multiboot2_to_bootparams(uint64_t mb_info_phys,
                                    BootParams* bp,
                                    VbeModeInfo* fb)
{
    // TEMP: just parse memory map later
    (void)mb_info_phys;
    (void)bp;
    (void)fb;
}

void setup_identity_paging(BootParams* bp, VbeModeInfo* fb)
{
    (void)bp;
    (void)fb;
}

void setup_stack(void)
{
    // Stack already set in ASM for now
}
// void early_kernel_main(uint64_t mb_info_phys)
// {
//     BootParams bootParams = {0};
//     VbeModeInfo fb_info = {0};

//     parse_multiboot2_to_bootparams(mb_info_phys, &bootParams, &fb_info);

//     setup_identity_paging(&bootParams, &fb_info);
//     setup_stack();

//     start(&bootParams, &fb_info);
// }




void parse_multiboot2_memory_map(multiboot2_info_t* mbi)
{
    uint8_t* tag_ptr = mbi->tags;

    log_info("Boot", "parse_multiboot2_memory_map tag_ptr = %p", tag_ptr);
    log_info("Boot", "parse_multiboot2_memory_map mbi->total_size = %u", mbi->total_size);


    while ((uintptr_t)tag_ptr < (uintptr_t)mbi + mbi->total_size)
    {
        multiboot2_tag_header_t* tag = (multiboot2_tag_header_t*)tag_ptr;

        // log_info("Boot", "parse_multiboot2_memory_map tag->type = %u", tag->type);


        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            multiboot2_mmap_entry_t* mmap = (multiboot2_mmap_entry_t*)(tag + 1);
            uintptr_t end = (uintptr_t)tag + tag->size;

            for (; (uintptr_t)mmap < end; mmap = (multiboot2_mmap_entry_t*)((uintptr_t)mmap + mmap->size)) {
                log_info("MEM", "Memory region: 0x%llx - 0x%llx type=%u",
                    mmap->entry_addr,
                    mmap->entry_addr + mmap->entry_len,
                    mmap->mem_type);
            }
        }

        // Move to next tag (8-byte aligned)
        tag_ptr = (uint8_t*)tag + ((tag->size + 7) & ~7);
    }
}


void parse_multiboot2_framebuffer(multiboot2_info_t* mbi) {
    uint8_t* tag_ptr = mbi->tags;

    while ((uintptr_t)tag_ptr < (uintptr_t)mbi + mbi->total_size) {
        multiboot2_tag_header_t* tag = (multiboot2_tag_header_t*)tag_ptr;

        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
            multiboot2_fb_tag_t* fb = (multiboot2_fb_tag_t*)(tag + 1);
            log_info("FB", "Framebuffer at 0x%llx %ux%u pitch=%u bpp=%u",
                     fb->addr, fb->width, fb->height, fb->pitch, fb->bpp);
        }

        tag_ptr = (uint8_t*)tag + ((tag->size + 7) & ~7);
    }
}


// typedef struct {
//     uint32_t total_size;
//     uint32_t reserved;
//     uint8_t  tags[];
// } multiboot2_info_t;

// void early_kernel_main(BootParams* bootParams, VbeModeInfo* fb_info)
// void early_kernel_main(void)
void early_kernel_main(void* multiboot_info)
{
    log_info("Boot", "early_kernel_main entered");

    multiboot2_info_t* mbi = (multiboot2_info_t*)multiboot_info;
    log_info("Boot", "mbi=%p total_size=%u", mbi, mbi->total_size);
    // log_info("Boot", "early_kernel_main total_size = %u, reserved = %u, tags = %u", mbi->total_size, mbi->reserved, mbi->tags);
    log_info("Boot", "total_size=%u, reserved=%u, tags=%p", mbi->total_size, mbi->reserved, (void*)mbi->tags);

    serial_putc_asm('1');
    serial_init();
    serial_putc_asm('2');


    parse_multiboot2_memory_map(mbi);
    parse_multiboot2_framebuffer(mbi);


    serial_putc_asm('B');

    debug_print_cs();
    serial_write("EARLY\n");
    serial_putc_asm('R');

    x64_IDT_Initialize();
    // HAL_Initialize();

    
    // serial_putc_asm('N');

    // serial_write("BEFORE UD2\n");
    // serial_putc_asm('A');

    // __asm__ volatile("ud2");
    // serial_putc_asm('L');

    // serial_write("AFTER UD2\n");

    for (;;) {
        __asm__ volatile("hlt");
    }
}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    log_info("Main", "Kernel Started");
    pmm_init(&bootParams->Memory);

    paging_bootstrap_identity(); // sets up kernel_page_directory

    for (uintptr_t pa = 0; pa < 32 * 1024 * 1024; pa += PAGE_SIZE)
    {
        map_identity_page(kernel_page_directory, pa);
    }

    uintptr_t stack_top_aligned = (uintptr_t)&_kernel_stack_top;
    stack_top_aligned = (stack_top_aligned + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    __asm__ volatile(
        "mov %0, %%rsp\n"
        :
        : "r"(stack_top_aligned)
        : "memory"
    );


    // map_vga
    for (uintptr_t pa = VGA_PHYS; pa < VGA_PHYS + 0x1000; pa += PAGE_SIZE)
    {
        map_identity_page(kernel_page_directory, pa);
    }

    // load page directory and enable paging
    write_cr3(kernel_page_directory_phys);
    enable_paging();

    paging_map_high_half_kernel(); // map high-half

    // _init();         // global constructors
    HAL_Initialize();

    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
        log_info("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }

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
end:
    // compositor_init();
    // // Launch compositor
    // compositor_main();  // infinite loop

    // log_info("Main", "Starting start_userspace!");
    start_userspace(bootParams);

    // should never come here
    log_critical("Main", "CRITICAL ERROR: User Space not started!");

    for (;;);
}