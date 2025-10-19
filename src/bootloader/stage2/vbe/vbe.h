#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <boot/bootparams.h>


typedef struct {
    char VbeSignature[4];             // == "VESA"
    uint16_t VbeVersion;                 // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];            // isa vbeFarPtr
    uint8_t Capabilities[4];
    uint32_t VideoModePtr;         // isa vbeFarPtr
    uint16_t TotalMemory;             // as # of 64KB blocks
    uint8_t _Reserved[236+256];
} __attribute__((packed)) VbeInfoBlock;

bool VBE_GetControllerInfo(VbeInfoBlock* info);
bool VBE_GetModeInfo(uint16_t mode, VbeModeInfo* info);
bool VBE_SetMode(uint16_t mode);
