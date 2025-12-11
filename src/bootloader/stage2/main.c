#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "mbr.h"
#include "stdlib.h"
#include "string.h"
#include "elf.h"
#include "memdetect.h"
#include <boot/bootparams.h>
#include "vbe/vbe.h"


uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

BootParams g_BootParams;

typedef void (*KernelStart)(BootParams* bootParams, VbeModeInfo* fbInfo);

void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition)
{
    clrscr();

    printf("Bootloader starting\n");

    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        goto end;
    }
    printf("Bootloader DISK_Initialize,partition = %p\n", partition);


    Partition part;
    MBR_DetectPartition(&part, &disk, partition);
    printf("Bootloader MBR_DetectPartition\n");

    if (!FAT_Initialize(&part))
    {
        printf("FAT init error\r\n");
        goto end;
    }
    printf("Bootloader FAT_Initialize\n");

    // prepare boot params
    g_BootParams.BootDevice = bootDrive;
    Memory_Detect(&g_BootParams.Memory);
    printf("Bootloader Memory_Detect\n");

    // load kernel
    KernelStart kernelEntry;
    if (!ELF_Read(&part, "/boot/kernel.elf", (void**)&kernelEntry))
    {
        printf("ELF read failed, booting halted!");
        goto end;
    }
    printf("Bootloader ELF_Read for KernelStart\n");

    // TODO: Check the value.
    const int desiredWidth = 1024;
    const int desiredHeight = 768;
    const int desiredBpp = 32;
    uint16_t pickedMode = 0xffff;

    // Initialize graphics 
    VbeInfoBlock* info = (VbeInfoBlock*)MEMORY_VESA_INFO;
    VbeModeInfo* modeInfo = (VbeModeInfo*)MEMORY_MODE_INFO;
    if (VBE_GetControllerInfo(info)) {
        uint16_t* mode = (uint16_t*)(info->VideoModePtr);
        for (int i = 0; mode[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(mode[i], modeInfo)) {
                printf("Can't get mode info %x :(\n", mode[i]);
                continue;
            }
            bool hasFB = (modeInfo->attributes & 0x90) == 0x90;

            if (hasFB && modeInfo->width == desiredWidth && modeInfo->height == desiredHeight && modeInfo->bpp == desiredBpp) {
                pickedMode = mode[i];
                break;
            }
        }

        // Set the VBE
        if (pickedMode != 0xFFFF && VBE_SetMode(pickedMode)) {
            printf("VBE mode set..(\n");
        }
    }
    else {
        printf("No VBE extensions :(\n");
    }

    // execute kernel
    kernelEntry(&g_BootParams, modeInfo);

end:
    for (;;);
}
