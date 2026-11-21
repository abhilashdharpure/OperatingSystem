#include <stdint.h>
#include "string.h"
#include "debug.h"
#include "paging.h"
#include "pmm.h"
#include "boot_paging.h"                       /* DIRECT_MAP_BASE */
#include "io.h"

typedef struct { uint8_t bus, dev, fn; uint16_t vendor, device; uint8_t cls, sub, prog; } pci_dev_t;
uint32_t pci_read32(uint8_t, uint8_t, uint8_t, uint8_t);
void     pci_write32(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);
int      pci_find(uint16_t, uint16_t, pci_dev_t *);
extern uint64_t *kernel_pml4_virt;

#define MMIO_VA 0xFFFFFFFFD8000000ULL          /* check that nothing else uses this range; DRM uses 0xC4000000..0xD4000000 */

enum { R_CTRL = 0x0000, R_STATUS = 0x0008, R_ICR = 0x00C0, R_IMC = 0x00D8, R_RCTL = 0x0100, R_TCTL = 0x0400,
       R_TIPG = 0x0410, R_RDBAL = 0x2800, R_RDBAH = 0x2804, R_RDLEN = 0x2808, R_RDH = 0x2810, R_RDT = 0x2818,
       R_TDBAL = 0x3800, R_TDBAH = 0x3804, R_TDLEN = 0x3808, R_TDH = 0x3810, R_TDT = 0x3818,
       R_MTA = 0x5200, R_RAL0 = 0x5400, R_RAH0 = 0x5404 };

#define NRX 32
#define NTX 8
struct rx_desc { uint64_t addr; uint16_t len, csum; uint8_t status, errors; uint16_t special; } __attribute__((packed));
struct tx_desc { uint64_t addr; uint16_t len; uint8_t cso, cmd, status, css; uint16_t special; } __attribute__((packed));

static volatile uint8_t *mmio;
static struct rx_desc *rx;
static struct tx_desc *tx;
static uint8_t *rxbuf[NRX], *txbuf[NTX];
uint8_t g_mac[6];

typedef struct net_device
{
    char name[16];

    uint8_t mac[6];

    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t dns[4];

    int link_up;
} net_device_t;

net_device_t g_netdev;

static inline uint32_t R(uint32_t r) { return *(volatile uint32_t *)(mmio + r); }
static inline void W(uint32_t r, uint32_t v) { *(volatile uint32_t *)(mmio + r) = v; }

static void *dma_page(uint64_t *phys)          /* pmm pages are below 512 MiB, so the direct map reaches them */
{
    uint64_t pa = pmm_alloc_page();
    if (!pa) return 0;
    *phys = pa;
    void *va = (void *)(DIRECT_MAP_BASE + pa);
    memset(va, 0, 4096);
    return va;
}

dhcp_assign_ip(
    uint8_t ip[4],
    uint8_t gateway[4],
    uint8_t dns[4])
{
    memcpy(g_netdev.ip,      ip,      4);
    memcpy(g_netdev.gateway, gateway, 4);
    memcpy(g_netdev.dns,     dns,     4);
}

static int text_read(struct file *f, void *buf, size_t size, const char *txt, size_t len)
{
    if (f->position >= len) return 0;
    size_t n = len - f->position;
    if (n > size) n = size;
    memcpy(buf, txt + f->position, n);
    f->position += n;
    return (int)n;
}

static int network_read(struct file *f,
                        void *buf,
                        size_t size)
{
    char t[256];

    ksnprintf(
        t,
        sizeof(t),
        "MAC=%d:%d:%d:%d:%d:%d\n"
        "IP=%d.%d.%d.%d\n"
        "Gateway=%d.%d.%d.%d\n"
        "Interface=%d.%d.%d.%d\n",
        g_netdev.mac[0],
        g_netdev.mac[1],
        g_netdev.mac[2],
        g_netdev.mac[3],
        g_netdev.mac[4],
        g_netdev.mac[5],

        g_netdev.ip[0],
        g_netdev.ip[1],
        g_netdev.ip[2],
        g_netdev.ip[3],

        g_netdev.gateway[0],
        g_netdev.gateway[1],
        g_netdev.gateway[2],
        g_netdev.gateway[3],

        g_netdev.dns[0],
        g_netdev.dns[1],
        g_netdev.dns[2],
        g_netdev.dns[3]
    );

    return text_read(
        f,
        buf,
        size,
        t,
        strlen(t));
}

static struct file_operations network_fops = { .read = network_read };

int e1000_init(void)
{


    VFS_RegisterDevice("/proc/net/info", &network_fops, NULL);

    pci_dev_t d;
    if (pci_find(0x8086, 0x100E, &d)) { log_info("E1000", "no 82540EM found"); return -1; }

    uint32_t cmd = pci_read32(d.bus, d.dev, d.fn, 0x04);
    pci_write32(d.bus, d.dev, d.fn, 0x04, (cmd & 0xFFFF) | 0x6);          /* memory space + bus master */

    uint64_t bar0 = pci_read32(d.bus, d.dev, d.fn, 0x10) & ~0xFULL;
    for (int i = 0; i < 32; i++)                                          /* 128 KiB of registers, uncached */
        if (map_page(kernel_pml4_virt, MMIO_VA + i * 4096ULL, bar0 + i * 4096ULL,
                     PAGE_PRESENT | PAGE_RW | (1ULL << 4) | (1ULL << 3)) != 0) {
            log_error("E1000", "cannot map BAR0"); return -1;
        }
    mmio = (volatile uint8_t *)MMIO_VA;

    W(R_IMC, 0xFFFFFFFFu);                                                /* polled: no interrupts */
    W(R_CTRL, R(R_CTRL) | (1u << 26));                                    /* reset */
    for (volatile int i = 0; i < 1000000 && (R(R_CTRL) & (1u << 26)); i++) { }
    W(R_IMC, 0xFFFFFFFFu); (void)R(R_ICR);
    W(R_CTRL, R(R_CTRL) | (1u << 6) | (1u << 5));                         /* set link up, auto speed */

    uint32_t ral = R(R_RAL0), rah = R(R_RAH0);
    g_mac[0] = ral; g_mac[1] = ral >> 8; g_mac[2] = ral >> 16; g_mac[3] = ral >> 24;
    g_mac[4] = rah; g_mac[5] = rah >> 8;
    W(R_RAH0, rah | (1u << 31));                                          /* address valid */
    for (int i = 0; i < 128; i++) W(R_MTA + i * 4, 0);

    uint64_t pa;
    rx = dma_page(&pa);  uint64_t rx_pa = pa;
    tx = dma_page(&pa);  uint64_t tx_pa = pa;
    if (!rx || !tx) return -1;
    for (int i = 0; i < NRX; i += 2) {                                    /* two 2 KiB buffers per page */
        uint64_t bpa; uint8_t *p = dma_page(&bpa);
        if (!p) return -1;
        for (int j = 0; j < 2; j++) { rxbuf[i + j] = p + j * 2048; rx[i + j].addr = bpa + j * 2048; }
    }
    for (int i = 0; i < NTX; i += 2) {
        uint64_t bpa; uint8_t *p = dma_page(&bpa);
        if (!p) return -1;
        for (int j = 0; j < 2; j++) { txbuf[i + j] = p + j * 2048; tx[i + j].addr = bpa + j * 2048; tx[i + j].status = 1; }
    }

    W(R_RDBAL, (uint32_t)rx_pa); W(R_RDBAH, rx_pa >> 32); W(R_RDLEN, NRX * 16);
    W(R_RDH, 0); W(R_RDT, NRX - 1);
    W(R_RCTL, (1u << 1) | (1u << 15) | (1u << 26));                       /* enable, broadcast accept, strip CRC, 2 KiB */
    W(R_TDBAL, (uint32_t)tx_pa); W(R_TDBAH, tx_pa >> 32); W(R_TDLEN, NTX * 16);
    W(R_TDH, 0); W(R_TDT, 0);
    W(R_TIPG, 0x0060200A);
    W(R_TCTL, (1u << 1) | (1u << 3) | (0x10u << 4) | (0x40u << 12));     /* enable, pad short packets */

    for(int i = 0; i < 6; ++i)
    {
        g_netdev.mac[i] = g_mac[i];
    }
    log_info("E1000", "MAC %x:%x:%x:%x:%x:%x link=%s", g_mac[0], g_mac[1], g_mac[2], g_mac[3], g_mac[4], g_mac[5],
             (R(R_STATUS) & 2) ? "up" : "down");
    return 0;
}

int e1000_send(const void *frame, uint16_t len)
{
    uint32_t t = R(R_TDT);
    struct tx_desc *d = &tx[t];
    if (!(d->status & 1) || len > 1514) return -1;                        /* ring full or too big */
    memcpy(txbuf[t], frame, len);
    d->len = len; d->cmd = 0x0B; d->status = 0;                           /* end of packet, add CRC, report status */
    __sync_synchronize();
    W(R_TDT, (t + 1) % NTX);
    return 0;
}

int e1000_poll(void *out, uint16_t max)                                   /* returns frame length, or 0 */
{
    uint32_t i = (R(R_RDT) + 1) % NRX;
    struct rx_desc *d = &rx[i];
    if (!(d->status & 1)) return 0;
    uint16_t n = d->len < max ? d->len : max;
    memcpy(out, rxbuf[i], n);
    d->status = 0;
    __sync_synchronize();
    W(R_RDT, i);
    return n;
}

/* Self-test: ask who has 10.0.2.2 (QEMU's gateway) and print its MAC. */
void e1000_arp_selftest(void)
{
    uint8_t f[60] = { 0 }, in[2048];
    memset(f, 0xFF, 6); memcpy(f + 6, g_mac, 6); f[12] = 0x08; f[13] = 0x06;
    f[14] = 0; f[15] = 1; f[16] = 8; f[17] = 0; f[18] = 6; f[19] = 4; f[21] = 1;       /* request */
    memcpy(f + 22, g_mac, 6); f[28] = 10; f[29] = 0; f[30] = 2; f[31] = 15;            /* sender 10.0.2.15 */
    f[38] = 10; f[39] = 0; f[40] = 2; f[41] = 2;                                       /* target 10.0.2.2 */
    if (e1000_send(f, 60)) { log_error("E1000", "send failed"); return; }
    for (int tries = 0; tries < 3000000; tries++) {
        int n = e1000_poll(in, sizeof in);
        if (n >= 42 && in[12] == 0x08 && in[13] == 0x06 && in[21] == 2) {
            log_info("E1000", "ARP reply: 10.0.2.2 is at %x:%x:%x:%x:%x:%x", in[22], in[23], in[24], in[25], in[26], in[27]);
            return;
        }
        (void)inb(0x80);
    }
    log_error("E1000", "no ARP reply: check /tmp/net.pcap");
}