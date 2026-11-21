#pragma once

#include <stdint.h>

#include "arch/x86_64/pci.h"

#define RTL8723DE_VENDOR_ID 0x10EC
#define RTL8723DE_DEVICE_ID 0xD723

typedef enum
{
    RTL_WIFI_OFFLINE = 0,
    RTL_WIFI_DETECTED,
    RTL_WIFI_INITIALIZING,
    RTL_WIFI_FIRMWARE_READY,
    RTL_WIFI_READY,
    RTL_WIFI_ERROR
} rtl_wifi_state_t;

typedef struct
{
    pci_dev_t pci;

    uintptr_t mmio_base;
    uint64_t mmio_phys;
    size_t mmio_size;

    uint8_t mac_address[6];

    rtl_wifi_state_t state;

    int initialized;
    int firmware_loaded;
    int radio_enabled;

} rtl8723de_device_t;


void pci_debug_root(void);
void rtl8723de_pci_debug(void);
int rtl8723de_init(void);
int rtl8723de_scan(void);
