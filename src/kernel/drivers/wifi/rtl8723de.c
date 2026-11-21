#include "rtl8723de.h"

#include "debug.h"
#include "arch/x86_64/pci.h"
#include "arch/x86_64/xhci.h"

#define RTL_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0xD723

static rtl8723de_device_t g_rtl8723de;

void pci_debug_root(void)
{
    uint32_t id;
    uint32_t addr;

    // Check PCI host bridge: 00:00.0
    addr = 0x80000000;

    outl(0xCF8, addr);

    uint32_t cf8 = inl(0xCF8);
    id = inl(0xCFC);

    log_info("PCI", "CF8 readback = %x", cf8);
    log_info("PCI", "Root bridge ID = %x", id);

    // Check another common location
    outl(0xCF8, 0x80000800);
    id = inl(0xCFC);

    log_info("PCI", "00:01.0 ID = %x", id);
}


void rtl8723de_pci_debug(void)
{
    uint32_t id;
    uint32_t class_code;
    uint32_t bar0;

    log_info("RTL8723DE", "Reading PCI configuration...");

    id = pci_read32(3, 0, 0, 0x00);
    class_code = pci_read32(3, 0, 0, 0x08);
    bar0 = pci_read32(3, 0, 0, 0x10);

    log_info("RTL8723DE", "PCI ID = %x", id);
    log_info("RTL8723DE", "CLASS = %x", class_code);
    log_info("RTL8723DE", "BAR0 = %x", bar0);

    if (id == 0xD72310EC)
    {
        log_info("RTL8723DE", "RTL8723DE FOUND!");
    }
    else
    {
        log_info("RTL8723DE", "Unexpected PCI ID");
    }
}



int rtl8723de_init(void)
{
    pci_dev_t dev;

    if (pci_find(
            RTL8723DE_VENDOR_ID,
            RTL8723DE_DEVICE_ID,
            &dev) != 0)
    {
        log_error("RTL8723DE", "Device not found");
        return -1;
    }

    g_rtl8723de.pci = dev;

    log_info("RTL8723DE",
             "Found at %02x:%02x.%u",
             dev.bus, dev.dev, dev.fn);

    /*
     * Read PCI BAR0.
     */

    uint32_t bar0 = pci_read32(
        dev.bus, dev.dev, dev.fn, 0x10);

    if (bar0 & 1)
    {
        log_error("RTL8723DE",
                  "Unexpected I/O BAR");
        return -1;
    }

    /*
     * Check whether BAR0 is 32-bit or 64-bit.
     * A 64-bit BAR requires reading BAR1 too.
     */

    uint64_t bar_phys = bar0 & ~0xFULL;

    if ((bar0 & 0x6) == 0x4)
    {
        uint32_t bar1 = pci_read32(
            dev.bus, dev.dev, dev.fn, 0x14);

        bar_phys |= ((uint64_t)bar1 << 32);
    }

    if (!bar_phys)
        return -1;

    /*
     * Enable PCI memory space and bus mastering.
     */

    uint32_t command = pci_read32(
        dev.bus, dev.dev, dev.fn, 0x04);

    uint16_t cmd = (uint16_t)command;

    cmd |= (1 << 1);  // Memory Space Enable
    cmd |= (1 << 2);  // Bus Master Enable

    pci_write32(
        dev.bus, dev.dev, dev.fn,
        0x04, (uint32_t)cmd);

    /*
     * Implement pci_map_bar() to determine the
     * actual BAR size and map its MMIO region.
     *
     * Do not assume that the BAR size is 4 KB.
     */

    g_rtl8723de.mmio_phys = bar_phys;

    if (pci_map_bar(
            &dev, 0, &g_rtl8723de.mmio_base,
            &g_rtl8723de.mmio_size) != 0)
    {
        log_error("RTL8723DE", "BAR mapping failed");
        return -1;
    }

    g_rtl8723de.state = RTL_WIFI_DETECTED;

    log_info("RTL8723DE",
             "PCI initialized. MMIO=%p size=%u",
             (void *)g_rtl8723de.mmio_base,
             (unsigned)g_rtl8723de.mmio_size);

    return 0;
}