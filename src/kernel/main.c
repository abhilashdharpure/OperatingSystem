#include <stdint.h>
#include "stdio.h"
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
#include <paging.h>

#include "hal/block.h"
#include "hal/fat32.h"

#include "boot_paging.h"
// #include "arch/x86_64/paging_bootstrap.h"

extern uint8_t _kernel_stack_top;

#define VGA_PHYS 0xB8000

void crash_me();

extern uint32_t mb_info_ptr;

BootParams   g_bootParams;
static VbeModeInfo  g_fbInfo;

extern uint64_t pml4_table[];      // defined in paging_tables.asm
extern uint64_t *kernel_pml4_virt; // defined in paging_bootstrap.c
extern uint64_t  kernel_pml4_phys; // defined in paging_bootstrap.c


// Data-only section
__attribute__((section(".boot64_stub_data")))
static BootParams bootparams_low;

/* reasonable upper bound */
#define MAX_MEMORY_REGIONS 64
__attribute__((section(".boot64_stub_data")))
static MemoryRegion g_memoryRegions[MAX_MEMORY_REGIONS];


__attribute__((section(".boot64_stub_data")))
static VbeModeInfo fb_low;

// FOr Framebuffer
#define KERNEL_FB_VA 0xFFFFFFFFC0000000ULL

void start(BootParams* bootParams, VbeModeInfo* fb_info);

void timer(ISRFrame64* regs)
{
    printf(".");
}

void test_read_sector0(block_device_t *disk) {
    if (!disk || !disk->read_sectors) {
        log_error("TEST", "No disk or read_sectors function");
        return;
    }

    uint8_t buffer[512];
    log_debug("TEST", "Trying to read LBA 0 from disk %s", disk->name);

    int ret = disk->read_sectors(disk, disk->lba_base + 0, 1, buffer);
    log_debug("TEST", "disk->read_sectors returned %d", ret);

    if (ret != 0) {
        log_error("TEST", "Failed to read sector 0");
        return;
    }

    log_debug("TEST", "Sector 0 first 16 bytes:");
    for (int i = 0; i < 16; i++) {
        log_debug("TEST", "%02x ", buffer[i]);
    }

    // Check MBR signature
    log_debug("TEST", "MBR signature: 0x%02x 0x%02x", buffer[510], buffer[511]);
}

void init_filesystem(void)
{
    log_debug("Main", "init_filesystem start");

    block_init();       // registers "sda" and initializes ATA
    register_first_fat32_partition();   // registers "sda1"

    block_device_t *part = block_lookup_by_name("sda1");
    if (!part) {
        log_error("MAIN", "No sda1 partition device found");
        return;
    }

    fat32_t *fs = fat32_init_device(part);
    if (!fs) {
        log_error("MAIN", "FAT32 init failed on sda1");
        return;
    }

    VFS_Mount("/", get_fat32_fops(), fs);
    log_info("MAIN", "FAT32 mounted at / from sda1");
}

void start_userspace(BootParams* bootParams)
{
    log_info("Main", "calling start_userspace");

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

    // Build argv for /bin/init
    static char *init_argv[] = {
        "init",
        NULL
    };
    static char *init_envp[] = {
        "PATH=/",
        NULL
    };
    // Hand off to ELF loader with argc=1, argv[0]="init"
    pid_t pid = exec_elf_mem(mf->data, mf->size, bootParams,
                             1, init_argv, init_envp);
    if (pid < 0)
    {
        panic("Failed to exec init");
    }
}

__attribute__((section(".boot64_stub")))
void test_serial_putc_asm(char c) {
    __asm__ volatile (
        "mov $0x3F8, %%dx\n\t"
        "mov %0, %%al\n\t"
        "out %%al, %%dx\n\t"
        :
        : "r"(c)
        : "dx", "al"
    );
}


__attribute__((section(".boot64_stub")))
void parse_multiboot2_memory_map(multiboot2_info_t* mbi, BootParams* out)
{
    uint8_t* tag_ptr = mbi->tags;
    uintptr_t mbi_end = (uintptr_t)mbi + mbi->total_size;
    out->Memory.RegionCount = 0;

    out->Memory.Regions = g_memoryRegions;
    while ((uintptr_t)tag_ptr < mbi_end)
    {
        multiboot2_tag_header_t* tag =
            (multiboot2_tag_header_t*)tag_ptr;

        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            multiboot2_tag_mmap_t* mmap =
                (multiboot2_tag_mmap_t*)tag;

            uintptr_t entry = (uintptr_t)mmap + sizeof(*mmap);
            uintptr_t end   = (uintptr_t)mmap + mmap->size;

            while (entry < end)
            {
                multiboot2_mmap_entry_t* e =
                    (multiboot2_mmap_entry_t*)entry;

                if (out->Memory.RegionCount < MAX_MEMORY_REGIONS)
                {
                    MemoryRegion* r =
                        &out->Memory.Regions[out->Memory.RegionCount++];
                    r->Begin  = e->addr;
                    r->Length = e->len;
                    r->Type   = e->type;
                    r->ACPI   = 0;
                }

                entry += mmap->entry_size;
            }
        }

        tag_ptr += (tag->size + 7) & ~7;
    }
}

__attribute__((section(".boot64_stub")))
void parse_multiboot2_framebuffer(multiboot2_info_t* mbi, VbeModeInfo* fb)
{
    uint8_t* tag_ptr = mbi->tags;
    uintptr_t mbi_end = (uintptr_t)mbi + mbi->total_size;

    while ((uintptr_t)tag_ptr < mbi_end)
    {
        multiboot2_tag_header_t* tag =
            (multiboot2_tag_header_t*)tag_ptr;

        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
        {
            multiboot2_fb_tag_t* mfb =
                (multiboot2_fb_tag_t*)tag;

            fb->framebuffer = (uintptr_t)mfb->addr;
            fb->width   = mfb->width;
            fb->height  = mfb->height;
            fb->pitch   = mfb->pitch;
            fb->bpp     = mfb->bpp;

            /* RGB defaults (Multiboot does not expose masks cleanly) */
            fb->red_mask   = 16;
            fb->green_mask = 8;
            fb->blue_mask  = 0;

            // log_info("FB",
            //     "Framebuffer at %p %ux%u pitch=%u bpp=%u",
            //     fb->framebuffer,
            //     fb->width,
            //     fb->height,
            //     fb->pitch,
            //     fb->bpp);
            return;
        }

        tag_ptr += (tag->size + 7) & ~7;
    }
}


void map_framebuffer(VbeModeInfo* fb)
{
    uint64_t pa    = (uint64_t)fb->framebuffer;        // physical from GRUB
    uint64_t size  = fb->pitch * fb->height;           // bytes
    uint64_t pages = (size + 0xFFF) / 0x1000;          // round up

    for (uint64_t i = 0; i < pages; i++)
    {
        uint64_t va   = KERNEL_FB_VA + i * 0x1000;
        uint64_t pa_i = pa + i * 0x1000;

        int r = map_page(kernel_pml4_virt, va, pa_i, PAGE_PRESENT | PAGE_RW);
        if (r != 0) {
            log_critical("FB", "map_page failed: va=%llx pa=%llx err=%d", va, pa_i, r);
            break;
        }
    }

    fb->framebuffer = (uintptr_t)KERNEL_FB_VA;
}


void kernel_high_entry(BootParams* params)
{
    log_info("Main", "Hello from higher half");
    // here you can safely use log_info, start(), etc.

    g_bootParams = *params; 
    start(params, &fb_low);
}

__attribute__((section(".boot64_stub")))
static void print_hex64(uint64_t v)
{
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t nib = (v >> i) & 0xF;
        char c = (nib < 10) ? ('0' + nib) : ('A' + nib - 10);
        test_serial_putc_asm(c);
    }
}



extern uint8_t _boot_stub_end;

__attribute__((section(".boot64_stub")))
void kernel_main_entry(void* multiboot_info)
{
    test_serial_putc_asm('A');

    boot_pmm_init();
    test_serial_putc_asm('B');


    multiboot2_info_t* mbi = (multiboot2_info_t*)multiboot_info;

    test_serial_putc_asm('C');

    uint64_t *pml4 = pml4_table;
    test_serial_putc_asm('D');


    parse_multiboot2_memory_map(mbi, &bootparams_low);
    parse_multiboot2_framebuffer(mbi, &fb_low);
    test_serial_putc_asm('E');

    bootparams_low.BootDevice = 0;
    test_serial_putc_asm('F');

    setup_high_mappings(pml4, &bootparams_low);

    test_serial_putc_asm('K');
    print_hex64((uint64_t)&kernel_high_entry);

    switch_to_high_pml4(pml4);

    kernel_high_entry(&bootparams_low);
    for (;;) __asm__ volatile("hlt");
}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    log_info("Main", "Kernel Started");

    // paging_init_long_mode_globals();

    pmm_init(&bootParams->Memory);

    // for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    // {
    //     log_info("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
    //         bootParams->Memory.Regions[i].Begin,
    //         bootParams->Memory.Regions[i].Length,
    //         bootParams->Memory.Regions[i].Type);
    // }

    // _init();         // global constructors
    HAL_Initialize();

    // log_info("Main", "Kernel After HAL intialized");

    VFS_Init();  
    // log_info("Main", "Kernel After VFS_Init");

    init_filesystem();
    // log_info("Main", "Kernel After init_filesystem");

    map_framebuffer(&fb_low);
    // log_info("Boot", "after map_framebuffer");

    debug_dump_va_mapping(kernel_pml4_virt, KERNEL_FB_VA);

    fb_init(fb_info);
    gfx_init();            // initialize graphics layer

    log_info("MAIN", "Framebuffer ready: %ux%u", fb_dev.width, fb_dev.height);


    gfx_clear(COLOR_BLACK);              // clear screen
end:

    // Enable Interrupts
    __asm__ volatile("sti");
    log_info("HAL", "Interrupts enabled");

    // // test_mouse_keyboard_read();
    // compositor_init();
    // // Launch compositor
    // compositor_main();  // infinite loop

    log_info("Main", "Starting start_userspace!");
    start_userspace(bootParams);

    // should never come here
    log_critical("Main", "CRITICAL ERROR: User Space not started!");

    for (;;);
}