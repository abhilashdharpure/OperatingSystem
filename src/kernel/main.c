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

#include <stdio.h>

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

void timer(Registers* regs)
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

void init_serial()
{
    // Reset UART (same as before)
    outb(0x00, 0x3F9);      // Disable interrupts
    outb(0x80, 0x3FB);      // Enable DLAB
    outb(0x03, 0x3F8);      // Divisor low byte (38400 baud)
    outb(0x00, 0x3F9);      // Divisor high byte
    outb(0x03, 0x3FB);      // 8 bits, no parity, 1 stop bit
    outb(0xC7, 0x3FA);      // FIFO enabled, clear
    outb(0x0B, 0x3FC);      // IRQs disabled, RTS/DSR set
}


// static inline void serial_putc(char c) {
//     // Wait until transmit buffer empty
//     while (!(inb(0x3F8 + 5) & 0x20));
//     *((volatile char *)0x3F8) = c;
// }
// static inline void serial_putc(char c) {
//     // Wait until transmit buffer empty
//     while (!(inb(0x3F8 + 5) & 0x20))
//         ;

//     outb(0x3F8, c);
// }


static void serial_write(const char *s) {
    while (*s) serial_putc(*s++);
}

static inline void serial_putc_asm(char c) {
    __asm__ volatile (
        "mov $0x3F8, %%dx\n\t"
        "mov %0, %%al\n\t"
        "out %%al, %%dx\n\t"
        :
        : "r"(c)
        : "dx", "al"
    );
}


void early_kernel_main(void) {

    serial_putc_asm('K');
    init_serial();

    // Prove we are here in the most primitive way
    serial_write("EARLY\n");

    x64_IDT_Initialize();

    serial_write("BEFORE UD2\n");
    __asm__ volatile("ud2");
    serial_write("AFTER UD2\n");

    for (;;) {
        __asm__ volatile("hlt");
    }
}

// void early_kernel_main(void) {
//     init_serial();
//     log_info("Boot", "early_kernel_main entered");

//     x64_IDT_Initialize();

//     log_info("Boot", "Triggering UD2 to test IDT...");
//     __asm__ volatile("ud2");

//     for (;;) {
//         __asm__ volatile("hlt");
//     }
// }



// void start(void)
// {
//     log_info("Main", "Kernel Started");

//     uint64_t mb_info = (uint64_t)mb_info_ptr;
//     parse_multiboot2(mb_info);

//     // pmm_init(...);
//     // paging_init(...);

//     // continue boot...
// }

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