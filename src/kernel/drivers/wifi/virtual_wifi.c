#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "arch/x86_64/pci.h"
#include "wifi.h"
#include "arch/x86_64/io.h"

#define VWIFI_VENDOR_ID 0x1234
#define VWIFI_DEVICE_ID 0x0001

#define VWIFI_REG_ID       0x00
#define VWIFI_REG_VERSION  0x04
#define VWIFI_REG_CONTROL  0x08
#define VWIFI_REG_STATUS  0x0C
#define VWIFI_REG_COMMAND 0x10
#define VWIFI_REG_COUNT   0x14

#define VWIFI_REG_NETWORKS 0x100
#define VWIFI_NETWORK_SIZE 0x40

#define VWIFI_CMD_SCAN 1

#define VWIFI_STATUS_READY 1

static uint16_t wifi_io_base;
static int wifi_detected;

static inline uint32_t wifi_read(uint16_t reg)
{
    return inl(wifi_io_base + reg);
}

static inline void wifi_write(uint16_t reg, uint32_t value)
{
    outl(wifi_io_base + reg, value);
}




int virtual_wifi_init(void)
{
    pci_dev_t dev;

    wifi_detected = 0;

    if (pci_find(VWIFI_VENDOR_ID,
                 VWIFI_DEVICE_ID,
                 &dev) != 0)
    {
        log_info("VWIFI", "Virtual Wi-Fi device not found");
        return -1;
    }

    uint32_t bar0 = pci_read32(
        dev.bus, dev.dev, dev.fn, 0x10
    );

    if (!(bar0 & 1))
    {
        log_error("VWIFI", "BAR0 is not an I/O BAR");
        return -1;
    }

    wifi_io_base = (uint16_t)(bar0 & ~3u);

    // Enable PCI I/O space.
    uint32_t command_status = pci_read32(
        dev.bus, dev.dev, dev.fn, 0x04
    );

    uint16_t command = (uint16_t)command_status;
    command |= 1;

    pci_write32(
        dev.bus, dev.dev, dev.fn, 0x04,
        (uint32_t)command
    );

    uint32_t id = wifi_read(VWIFI_REG_ID);

    if (id != 0x57494649)
    {
        log_error("VWIFI", "Invalid hardware ID: %x", id);
        return -1;
    }

    wifi_write(VWIFI_REG_CONTROL, 1);

    wifi_detected = 1;

    log_info("VWIFI",
             "Virtual Wi-Fi initialized at PCI %02x:%02x.%u",
             dev.bus, dev.dev, dev.fn);

    log_info("VWIFI",
             "I/O Base: %x Version: %x",
             wifi_io_base,
             wifi_read(VWIFI_REG_VERSION));

    return 0;
}

int virtual_wifi_scan(void)
{
    if (!wifi_detected)
        return -1;

    log_info("VWIFI", "Starting Wi-Fi scan");

    wifi_write(VWIFI_REG_COMMAND, VWIFI_CMD_SCAN);

    // Bounded polling.
    for (uint32_t i = 0; i < 1000000; i++)
    {
        if (wifi_read(VWIFI_REG_STATUS) & VWIFI_STATUS_READY)
        {
            int count = wifi_read(VWIFI_REG_COUNT);

            log_info("VWIFI",
                     "Scan completed: %d networks",
                     count);

            return count;
        }
    }

    log_error("VWIFI", "Scan timeout");

    return -1;
}

int virtual_wifi_get_networks(
    wifi_network_t *networks,
    int max_networks)
{
    if (!wifi_detected || !networks || max_networks <= 0)
        return -1;

    int count = wifi_read(VWIFI_REG_COUNT);

    if (count > WIFI_MAX_NETWORKS)
        count = WIFI_MAX_NETWORKS;

    if (count > max_networks)
        count = max_networks;

    for (int i = 0; i < count; i++)
    {
        uint16_t base =
            VWIFI_REG_NETWORKS + i * VWIFI_NETWORK_SIZE;

        // Read SSID.
        for (int j = 0; j < 32; j += 4)
        {
            uint32_t word = wifi_read(base + j);

            networks[i].ssid[j]     = word;
            networks[i].ssid[j + 1] = word >> 8;
            networks[i].ssid[j + 2] = word >> 16;
            networks[i].ssid[j + 3] = word >> 24;
        }

        networks[i].ssid[32] = '\0';

        // Read BSSID.
        uint32_t bssid_low = wifi_read(base + 0x20);
        uint32_t bssid_high = wifi_read(base + 0x24);

        networks[i].bssid[0] = bssid_low;
        networks[i].bssid[1] = bssid_low >> 8;
        networks[i].bssid[2] = bssid_low >> 16;
        networks[i].bssid[3] = bssid_low >> 24;
        networks[i].bssid[4] = bssid_high;
        networks[i].bssid[5] = bssid_high >> 8;

        networks[i].signal =
            (int8_t)wifi_read(base + 0x28);

        networks[i].channel =
            wifi_read(base + 0x2C);

        networks[i].security =
            wifi_read(base + 0x30);
    }

    return count;
}
