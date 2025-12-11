#include "mbr.h"
#include "memory.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <stddef.h>

#define PARTITION_ENTRY_LINEAR_ADDR  0x20000UL

typedef struct {
    // 0x00	1	Drive attributes (bit 7 set = active or bootable)
    uint8_t attributes;

    // 0x01	3	CHS Address of partition start
    uint8_t chsStart[3];

    // 0x04	1	Partition type
    uint8_t partitionType;

    // 0x05	3	CHS address of last partition sector
    uint8_t chsEnd[3];

    // 0x08	4	LBA of partition start
    uint32_t lbaStart;

    // 0x0C	4	Number of sectors in partition
    uint32_t size;

} __attribute__((packed)) MBR_Entry;


// void MBR_DetectPartition(Partition* part, DISK* disk, void* partition)
// {
//     printf("MBR_DetectPartition Start\n");

//     part->disk = disk;
//     if (disk->id < 0x80)
//     {
//         printf("MBR_DetectPartition if (disk->id < 0x80)\n");

//         part->partitionOffset = 0;
//         part->partitionSize = (uint32_t)(disk->cylinders)
//              * (uint32_t)(disk->heads)
//              * (uint32_t)(disk->sectors);
//     }
//     else
//     {
//         printf("MBR_DetectPartition else\n");

//         MBR_Entry* entry = (MBR_Entry*)segoffset_to_linear(partition);

//         printf("MBR_DetectPartition entry->lbaStart = %u \n", (unsigned)entry->lbaStart);
//         printf("MBR_DetectPartition entry->size = %u \n", (unsigned)entry->size);

//         part->partitionOffset = entry->lbaStart;
//         part->partitionSize = entry->size;
//     }
// }


void MBR_DetectPartition(Partition* part, DISK* disk, void* partition)
{
    part->disk = disk;

    if (disk->id < 0x80) {
        part->partitionOffset = 0;
        part->partitionSize = (uint32_t)(disk->cylinders)
             * (uint32_t)(disk->heads)
             * (uint32_t)(disk->sectors);
        return;
    }

    MBR_Entry* entry_from_param = NULL;
    MBR_Entry* entry_from_fixed = (MBR_Entry*)PARTITION_ENTRY_LINEAR_ADDR;

    // Try to use the 'partition' pointer if it looks sensible; else fall back
    if (partition != NULL) {
        // If 'partition' is a segmented pointer from stage1 you'll probably need
        // to convert it. If segoffset_to_linear exists you can try it:
        // entry_from_param = (MBR_Entry*) segoffset_to_linear(partition);
        // but many toolchains don't pass a proper far pointer — so be conservative:
        entry_from_param = (MBR_Entry*)partition;
    }

    // Basic sanity check: lbaStart should be a reasonable small value (not huge)
    if (entry_from_param != NULL && entry_from_param->lbaStart > 0 && entry_from_param->lbaStart < 0x00FFFFFF) {
        part->partitionOffset = entry_from_param->lbaStart;
        part->partitionSize   = entry_from_param->size;
        printf("MBR_DetectPartition: used entry_from_param lbaStart=%u size=%u\n",
               entry_from_param->lbaStart, entry_from_param->size);
    } else {
        // fallback to the known fixed location where stage1 copied it
        part->partitionOffset = entry_from_fixed->lbaStart;
        part->partitionSize   = entry_from_fixed->size;
        printf("MBR_DetectPartition: used fixed addr lbaStart=%u size=%u\n",
               entry_from_fixed->lbaStart, entry_from_fixed->size);
    }
}

bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void* lowerDataOut)
{
    return DISK_ReadSectors(part->disk, lba + part->partitionOffset, sectors, lowerDataOut);
}