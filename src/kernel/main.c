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
#include "drivers/input/input.h"
#include "drivers/display/display.h"
#include "compositor/compositor.h"
#include <arch/x86_64/io.h>
#include <arch/x86_64/msr.h>

#include "multiboot2_defines.h"
#include "arch/x86_64/serial.h"

#include "paging.h"
#include "pmm.h"
#include "arch/x86_64/gdt.h"
#include "hal/vfs.h"
#include "memfile.h"
#include "hal/partition.h"
#include "hal/drm.h"

#include <hal/elf.h>
#include <hal/process.h>
#include <hal/scheduler.h>
#include <paging.h>

#include "hal/block.h"
#include "hal/fat32.h"

#include "boot_paging.h"
// #include "arch/x86_64/paging_bootstrap.h"

extern uint8_t _kernel_stack_top;
extern char _kernel_end;

#define VGA_PHYS 0xB8000

void crash_me();

extern uint32_t mb_info_ptr;

BootParams   g_bootParams;
static VbeModeInfo  g_fbInfo;
// fb_device_t fb_dev;

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
uint32_t g_fb_mapped_bytes;


void start(BootParams* bootParams, VbeModeInfo* fb_info);

void timer(ISRFrame64* regs)
{
    printf(".");
}

uint32_t fb_mapped_bytes(void)
{
    return g_fb_mapped_bytes;
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
        boot_mark(4, 0x00B00000);
        return;
    }

    fat32_t *fs = fat32_init_device(part);
    if (!fs) {
        log_error("MAIN", "FAT32 init failed on sda1");
        return;
    }

    VFS_Mount("/", get_fat32_fops(), fs);
    log_info("MAIN", "FAT32 mounted at / from sda1");

    tmpfs_init();
    log_info("MAIN", "tmpfs_init");

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

            return;
        }

        tag_ptr += (tag->size + 7) & ~7;
    }
}

__attribute__((section(".boot64_stub_data")))
static uint64_t root_img_low[2];                       /* base, size */
uint64_t g_root_image_base, g_root_image_size;

__attribute__((section(".boot64_stub")))
void parse_multiboot2_module(multiboot2_info_t* mbi, uint64_t out[2])
{
    out[0] = 0;
    out[1] = 0;
    uint8_t* p = mbi->tags;
    uintptr_t end = (uintptr_t)mbi + mbi->total_size;
    while ((uintptr_t)p < end) {
        multiboot2_tag_header_t* t = (multiboot2_tag_header_t*)p;
        if (t->type == 0) return;                          /* end tag */
        if (t->type == 3) {                                /* module: { type, size, mod_start, mod_end, cmdline } */
            uint32_t* w = (uint32_t*)t;
            out[0] = w[2];
            out[1] = (uint64_t)w[3] - w[2];
            return;
        }
        p += (t->size + 7) & ~7;
    }
}

void map_framebuffer(VbeModeInfo* fb)
{
    uint64_t pa    = (uint64_t)fb->framebuffer;        // physical from GRUB
    uint32_t size = bochs_present() ? bochs_vram_bytes() : (uint32_t)(fb->pitch * fb->height);
    g_fb_mapped_bytes = size; 
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
    test_serial_putc_asm('L');

    log_info("Main", "Hello from higher half");
    // here you can safely use log_info, start(), etc.
    test_serial_putc_asm('M');

    g_bootParams = *params;
    g_root_image_base = root_img_low[0];
    g_root_image_size = root_img_low[1];
    g_bootParams.RootImageBase = root_img_low[0];
    g_bootParams.RootImageSize = root_img_low[1];


    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0,%0" : "=r"(cr0));
    __asm__ volatile("mov %%cr4,%0" : "=r"(cr4));
    log_info("CPU", "CR0=0x%llx WP=%d CR4=0x%llx EFER=0x%llx",
            (unsigned long long)cr0, (int)((cr0 >> 16) & 1),
            (unsigned long long)cr4, (unsigned long long)rdmsr(0xC0000080));
            
    __asm__ volatile("mov %%cr0,%0" : "=r"(cr0));
    cr0 &= ~(1ULL << 16);
    __asm__ volatile("mov %0,%%cr0" :: "r"(cr0) : "memory");

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


    uint64_t *pml4 = pml4_table;

    parse_multiboot2_memory_map(mbi, &bootparams_low);
    test_serial_putc_asm('C');

    parse_multiboot2_framebuffer(mbi, &fb_low);
    test_serial_putc_asm('D');

    parse_multiboot2_module(mbi, root_img_low);
    test_serial_putc_asm('E');

    if (root_img_low[1])
    {
        test_serial_putc_asm('F');
        boot_pmm_avoid(root_img_low[0], root_img_low[0] + root_img_low[1]);
        boot_pmm_skip_to((uintptr_t)(root_img_low[0] + root_img_low[1]));
    }
    test_serial_putc_asm('K');

    bootparams_low.BootDevice = 0;

    setup_high_mappings(pml4, &bootparams_low);
    if (root_img_low[1]) {                            /* give the module a direct mapping if it lies above 512 MiB */
        uint64_t me = root_img_low[0] + root_img_low[1];
        if (me > BOOT_PMM_LIMIT_PA)
        {
            boot_map_direct_range(pml4,
                root_img_low[0] > BOOT_PMM_LIMIT_PA ? root_img_low[0] : BOOT_PMM_LIMIT_PA, me);
        }
    }

    print_hex64((uint64_t)&kernel_high_entry);

    test_serial_putc_asm('L');

    switch_to_high_pml4(pml4);
    test_serial_putc_asm('M');

    kernel_high_entry(&bootparams_low);
    for (;;) __asm__ volatile("hlt");
}

void start(BootParams* bootParams, VbeModeInfo* fb_info)
{   
    log_info("Main", "Kernel Started");

    // paging_init_long_mode_globals();

    pmm_init(&bootParams->Memory);

    if (g_root_image_size) {
        uintptr_t kend = kernel_virt_to_phys(&_kernel_end);
        if (g_root_image_base < kend)
            log_error("MAIN", "root image 0x%llx overlaps kernel end 0x%llx: fix linker.ld",
                      (unsigned long long)g_root_image_base, (unsigned long long)kend);
    }

    if (g_root_image_size) {
        uintptr_t s = (uintptr_t)(g_root_image_base & ~0xFFFULL);
        pmm_reserve_region(s, (uintptr_t)(g_root_image_base + g_root_image_size - s));
        log_info("MAIN", "root image: phys [0x%llx, 0x%llx), boot_alloc_end=0x%llx",
                 (unsigned long long)g_root_image_base,
                 (unsigned long long)(g_root_image_base + g_root_image_size),
                 (unsigned long long)boot_get_boot_alloc_end());
    }
    
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

    bochs_init();  
    map_framebuffer(&fb_low);
    // log_info("Boot", "after map_framebuffer");

    debug_dump_va_mapping(kernel_pml4_virt, KERNEL_FB_VA);

    fb_init(fb_info);
    gfx_init();            // initialize graphics layer

    boot_mark(0, 0x00FFFFFF);

    VFS_Init();  
    // log_info("Main", "Kernel After VFS_Init");

    init_filesystem();
    // log_info("Main", "Kernel After init_filesystem");
    boot_mark(1, 0x0000FF00);

    display_devices_init();
    drm_init();

    battery_devices_init();

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

    boot_mark(2, 0x000000FF);

    log_info("Main", "Starting start_userspace!");
    start_userspace(bootParams);

    // should never come here
    log_critical("Main", "CRITICAL ERROR: User Space not started!");

    for (;;);
}