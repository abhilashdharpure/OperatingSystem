// xhci.c: xHCI controller start, device enumeration, HID boot mouse -> mouse_dev (polled, no interrupts)
#include "xhci.h"
#include <stdint.h>
#include "string.h"
#include "debug.h"
#include "paging.h"
#include "pmm.h"
#include "boot_paging.h"                       /* DIRECT_MAP_BASE */
#include "arch/x86_64/io.h"
#include "arch/x86_64/pit.h"

uint32_t pci_read32(uint8_t, uint8_t, uint8_t, uint8_t);
void     pci_write32(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);
int      pci_find_class(uint8_t, uint8_t, uint8_t, pci_dev_t *);
void     boot_bits(int row, uint8_t v, uint32_t on);
void     mouse_inject(int dx, int dy, uint8_t buttons);          /* ps2.c */
extern uint64_t *kernel_pml4_virt;

#define MMIO_VA     0xFFFFFFFFD9000000ULL                         /* check that nothing else uses this range */
#define MMIO_PAGES  32
#define PORTSC_RW1C 0x00FE0002u                                   /* PED and the change bits: write 1 to clear, so mask them out */

typedef struct { uint64_t param; uint32_t status; uint32_t control; } __attribute__((packed)) trb_t;
typedef struct { trb_t *trb; uint64_t pa; uint32_t enq, cycle; } ring_t;

static volatile uint8_t *cap, *op, *rt, *db;
static uint32_t cs;                                               /* context size: 32 or 64 bytes */
static uint64_t *dcbaa;
static ring_t cmdr, ep0s[256], hid_ring;
static volatile trb_t *evr;
static uint64_t evr_pa;
static uint32_t ev_deq, ev_ccs;
static uint8_t *xb;  static uint64_t xb_pa;                       /* scratch buffer for control transfers */
static int xhci_ready;

static int      hid_slot, hid_dci, hid_errors;                    /* the (first) HID boot mouse */
static uint32_t hid_len;
static uint8_t *hid_buf; static uint64_t hid_buf_pa;
static uint8_t  dbg_ok, dbg_err;                                  /* shown on screen: devices enumerated, last failing step */


static uint32_t nports_g;
static uint8_t  usb2_first, usb2_count;
static uint8_t  port_done[256];
static volatile int xhci_busy;
static int      hid_port;


#define FAIL(step, code) (dbg_err = (uint8_t)(((step) << 4) | ((code) & 15)))

static inline uint32_t R32(volatile uint8_t *b, uint32_t o) { return *(volatile uint32_t *)(b + o); }
static inline void W32(volatile uint8_t *b, uint32_t o, uint32_t v) { *(volatile uint32_t *)(b + o) = v; }
static inline void W64(volatile uint8_t *b, uint32_t o, uint64_t v) { W32(b, o, (uint32_t)v); W32(b, o + 4, (uint32_t)(v >> 32)); }
extern uint64_t pit_get_ticks(void);
extern uint64_t pit_get_frequency(void);

static uint64_t tsc_per_us;

static void tsc_calibrate(void)                    /* needs the timer interrupt: call after sti */
{
    uint64_t freq = pit_get_frequency(), guard = 0;
    uint64_t t0 = pit_get_ticks();
    while (pit_get_ticks() == t0)                  /* wait for a tick edge */
        if (++guard > 400000000ULL) { tsc_per_us = 2000; return; }       /* no timer: assume 2 GHz */
    uint64_t a = __builtin_ia32_rdtsc(), t1 = pit_get_ticks();
    uint64_t want = freq / 50 ? freq / 50 : 1;     /* about 20 ms of ticks */
    while (pit_get_ticks() - t1 < want) { }
    uint64_t b = __builtin_ia32_rdtsc();
    uint64_t us = want * 1000000ULL / freq;
    tsc_per_us = (b - a) / (us ? us : 1);
    if (!tsc_per_us) tsc_per_us = 2000;
    log_info("XHCI", "TSC about %llu MHz", (unsigned long long)tsc_per_us);
}

static void udelay(uint32_t us)
{
    if (!tsc_per_us) { for (volatile uint32_t i = 0; i < us * 20; i++) __asm__ volatile("pause"); return; }
    uint64_t end = __builtin_ia32_rdtsc() + (uint64_t)us * tsc_per_us;
    while (__builtin_ia32_rdtsc() < end) __asm__ volatile("pause");
}
static void msleep(uint32_t ms) { udelay(ms * 1000u); }

static int wait_for(volatile uint8_t *b, uint32_t o, uint32_t mask, uint32_t val, int ms)
{
    for (int i = 0; i < ms * 1000; i++) { if ((R32(b, o) & mask) == val) return 0; udelay(1); }
    return -1;
}


volatile uint8_t *portreg(uint32_t p)
{
    return op + 0x400 + 0x10 * (p - 1);
}
static void *dma_page(uint64_t *pa)                               /* zeroed page; pmm pages are under 512 MiB: the direct map reaches them */
{
    uint64_t p = pmm_alloc_page();
    if (!p) return 0;
    *pa = p;
    void *v = (void *)(DIRECT_MAP_BASE + p);
    memset(v, 0, 4096);
    return v;
}

/* ---- rings ---- */
static int ring_init(ring_t *r)
{
    uint64_t pa; trb_t *t = dma_page(&pa);
    if (!t) return -1;
    r->trb = t; r->pa = pa; r->enq = 0; r->cycle = 1;
    t[255].param = pa; t[255].control = (6u << 10) | (1u << 1);   /* link back to the start, toggle cycle */
    return 0;
}
static void ring_put(ring_t *r, uint64_t param, uint32_t status, uint32_t control)
{
    trb_t *t = &r->trb[r->enq];
    t->param = param; t->status = status;
    __sync_synchronize();
    t->control = control | r->cycle;                              /* cycle bit last: the controller sees a whole TRB */
    if (++r->enq == 255) {
        __sync_synchronize();
        r->trb[255].control = (6u << 10) | (1u << 1) | r->cycle;
        r->enq = 0; r->cycle ^= 1;
    }
}
static void ring_db(uint32_t slot, uint32_t target) { __sync_synchronize(); W32(db, slot * 4, target); }

/* ---- events ---- */
static int evt_next(trb_t *out)
{
    volatile trb_t *t = &evr[ev_deq];
    if ((t->control & 1) != ev_ccs) return 0;
    __sync_synchronize();
    out->param = t->param; out->status = t->status; out->control = t->control;
    if (++ev_deq == 256) { ev_deq = 0; ev_ccs ^= 1; }
    W64(rt + 0x20, 0x18, (evr_pa + ev_deq * 16ULL) | (1u << 3));   /* new dequeue pointer, clear the busy bit */
    return 1;
}

static void hid_event(const trb_t *t);

static int wait_cmd(trb_t *out)                                   /* the next command completion event */
{
    for (int i = 0; i < 1000000; i++) {
        trb_t ev;
        if (evt_next(&ev)) {
            uint32_t ty = (ev.control >> 10) & 0x3F;
            if (ty == 33) { *out = ev; return 0; }
            if (ty == 32) hid_event(&ev);
            continue;
        }
        udelay(1);
    }
    return -1;
}
static int wait_xfer(uint32_t slot, trb_t *out)                   /* a transfer event for EP0 of this slot */
{
    for (int i = 0; i < 1000000; i++) {
        trb_t ev;
        if (evt_next(&ev)) {
            if (((ev.control >> 10) & 0x3F) == 32) {
                if ((ev.control >> 24) == slot && ((ev.control >> 16) & 0x1F) == 1) { *out = ev; return 0; }
                hid_event(&ev);
            }
            continue;
        }
        udelay(1);
    }
    return -1;
}

static int command(uint64_t param, uint32_t control, trb_t *ev)  /* returns the completion code (1 = success) or -1 on timeout */
{
    ring_put(&cmdr, param, 0, control);
    ring_db(0, 0);
    if (wait_cmd(ev)) return -1;
    return (int)(ev->status >> 24);
}

static int control(uint32_t slot, ring_t *ep0, uint8_t rtype, uint8_t rq, uint16_t wv, uint16_t wi, uint16_t wl)
{
    int in = rtype & 0x80;
    uint64_t setup = rtype | ((uint64_t)rq << 8) | ((uint64_t)wv << 16) | ((uint64_t)wi << 32) | ((uint64_t)wl << 48);
    uint32_t trt = wl ? (in ? 3u : 2u) : 0u;
    if (wl) memset(xb, 0, wl < 4096 ? wl : 4096);
    ring_put(ep0, setup, 8, (2u << 10) | (1u << 6) | (trt << 16));                    /* setup stage, immediate data */
    if (wl) ring_put(ep0, xb_pa, wl, (3u << 10) | (in ? (1u << 16) : 0));            /* data stage */
    ring_put(ep0, 0, 0, (4u << 10) | (1u << 5) | ((wl && in) ? 0u : (1u << 16)));    /* status stage, interrupt on completion */
    ring_db(slot, 1);
    trb_t ev;
    if (wait_xfer(slot, &ev)) return -1;
    uint32_t code = ev.status >> 24;
    return (code == 1 || code == 13) ? 0 : -(int)code;
}

/* ---- the HID mouse ---- */
static void hid_event(const trb_t *t)
{
    uint32_t slot = t->control >> 24, epid = (t->control >> 16) & 0x1F;
    if (!hid_slot || slot != (uint32_t)hid_slot || epid != (uint32_t)hid_dci) return;
    uint32_t code = t->status >> 24;
    if (code == 1 || code == 13) {
        uint32_t got = hid_len - (t->status & 0xFFFFFF);
        if (got >= 3) mouse_inject((int8_t)hid_buf[1], (int8_t)hid_buf[2], hid_buf[0] & 7);
        hid_errors = 0;
    } else if (++hid_errors > 100) {
        log_info("XHCI", "mouse endpoint keeps failing (code %u): stopping", code);
        hid_slot = 0;
        return;
    }
    ring_put(&hid_ring, hid_buf_pa, hid_len, (1u << 10) | (1u << 5) | (1u << 2));    /* queue the next report */
    ring_db(hid_slot, hid_dci);
}

void xhci_service(void)                       /* call from a syscall context only: it may sleep while enumerating */
{
    static uint64_t last;
    if (!xhci_ready || xhci_busy) return;
    uint64_t now = pit_get_ticks();
    if (now - last < 50) return;               /* look about every 50 ms */
    last = now;

    for (uint32_t p = 1; p <= nports_g; p++) {
        uint32_t sc = R32(portreg(p), 0);
        if (!(sc & (1u << 17))) continue;                                     /* connect status change */
        W32(portreg(p), 0, (sc & ~PORTSC_RW1C) | (1u << 17));
        xhci_busy = 1;                                                        /* the timer must not consume events meanwhile */
        if (sc & 1) { port_done[p] = 0; msleep(150); enumerate_port(p); }
        else { port_done[p] = 0; if ((int)p == hid_port) { hid_slot = 0; hid_port = 0; log_info("XHCI", "mouse unplugged"); } }
        xhci_busy = 0;
    }
}

void xhci_poll(void)                                              /* call from the timer tick and the input read path */
{
    if (!xhci_ready || xhci_busy) return;
    unsigned long flags;
    __asm__ volatile("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
    trb_t ev;
    for (int n = 0; n < 16 && evt_next(&ev); n++)
        if (((ev.control >> 10) & 0x3F) == 32) hid_event(&ev);
    if (flags & 0x200) __asm__ volatile("sti");
}

/* ---- device enumeration ---- */
void enumerate_port(uint32_t p)
{
    volatile uint8_t *pr = op + 0x400 + 0x10 * (p - 1);
    uint32_t sc = R32(pr, 0);
    if (!(sc & 1)) return;                                        /* nothing connected */

    if (p < 256 && port_done[p])
    {
        return;
    }

    if (!is_usb2(p))
    {
        return;                       /* USB 3 ports link-train on their own: a USB 2 mouse uses its USB 2 companion port */
    }

    if (!(sc & 2))
    {                                              /* not enabled yet (USB 2 port): reset it */
        W32(pr, 0, (sc & ~PORTSC_RW1C) | (1u << 4));
        for (int i = 0; i < 500 && !(R32(pr, 0) & (1u << 21)); i++) 
        {
            msleep(1);
        }

        sc = R32(pr, 0);
        W32(pr, 0, (sc & ~PORTSC_RW1C) | (1u << 21) | (1u << 17));     /* clear reset-change and connect-change */
        msleep(20);
        sc = R32(pr, 0);
        if (!(sc & 2)) { FAIL(7, 0); log_info("XHCI", "port %u: not enabled after reset (PORTSC=0x%x)", p, sc); return; }
    }
    uint32_t speed = (sc >> 10) & 0xF;                            /* 1 full, 2 low, 3 high, 4 super */
    uint32_t mps = speed == 4 ? 512 : speed == 3 ? 64 : 8;

    trb_t ev;
    int code = command(0, 9u << 10, &ev);                         /* enable slot */
    if (code != 1) { FAIL(1, code); log_info("XHCI", "port %u: enable slot failed (%d)", p, code); return; }
    uint32_t slot = ev.control >> 24;
    if (!slot) return;

    uint64_t out_pa, in_pa;
    uint8_t *out = dma_page(&out_pa), *in = dma_page(&in_pa);
    ring_t *ep0 = &ep0s[slot];
    if (!out || !in || ring_init(ep0)) return;
    dcbaa[slot] = out_pa;

    ((uint32_t *)in)[1] = (1u << 0) | (1u << 1);                  /* add: slot context, EP0 */
    uint32_t *sl = (uint32_t *)(in + cs);
    sl[0] = (1u << 27) | (speed << 20);                           /* one context entry, speed */
    sl[1] = p << 16;                                              /* root hub port number */
    uint32_t *e0 = (uint32_t *)(in + 2 * cs);
    uint64_t trdp = ep0->pa | 1;                                  /* dequeue pointer + cycle state 1 */
    e0[1] = (3u << 1) | (4u << 3) | (mps << 16);                  /* error count 3, control endpoint */
    e0[2] = (uint32_t)trdp; e0[3] = (uint32_t)(trdp >> 32);
    e0[4] = 8;
    code = command(in_pa, (11u << 10) | (slot << 24), &ev);       /* address device */
    if (code != 1) { FAIL(2, code); log_info("XHCI", "port %u: address device failed (%d)", p, code); return; }

    if (control(slot, ep0, 0x80, 6, 0x0100, 0, 8)) { FAIL(3, 1); log_info("XHCI", "port %u: no device descriptor", p); return; }
    uint32_t m0 = xb[7];
    if (speed == 4) m0 = 1u << m0;
    if (m0 && m0 != mps && m0 <= 512) {                           /* the real EP0 packet size differs from my guess */
        memset(in, 0, 4096);
        ((uint32_t *)in)[1] = 1u << 1;
        memcpy(in + 2 * cs, out + cs, cs);
        uint32_t *e = (uint32_t *)(in + 2 * cs);
        e[1] = (e[1] & 0xFFFFu) | (m0 << 16);
        command(in_pa, (13u << 10) | (slot << 24), &ev);          /* evaluate context */
        mps = m0;
    }
    if (control(slot, ep0, 0x80, 6, 0x0100, 0, 18)) { FAIL(3, 2); return; }
    uint16_t vid = xb[8] | (xb[9] << 8), pid = xb[10] | (xb[11] << 8);
    log_info("XHCI", "port %u: slot %u speed %u device %x:%x class %x", p, slot, speed, vid, pid, xb[4]);
    dbg_ok++;

    if (control(slot, ep0, 0x80, 6, 0x0200, 0, 9)) { FAIL(4, 1); return; }
    uint32_t total = xb[2] | (xb[3] << 8);
    if (total < 9) return;
    if (total > 1024) total = 1024;
    if (control(slot, ep0, 0x80, 6, 0x0200, 0, (uint16_t)total)) { FAIL(4, 2); return; }

    uint8_t cfgval = xb[5];                                       /* find a HID boot-protocol mouse interface and its interrupt-in endpoint */
    int iface = -1; uint32_t ep_addr = 0, ep_mps = 0, ep_int = 0;
    int cur_if = -1, cur_ok = 0;
    for (uint32_t i = 0; i + 2 <= total; ) {
        uint8_t len = xb[i], type = xb[i + 1];
        if (len < 2) break;
        if (type == 4 && len >= 9) {
            cur_if = xb[i + 2];
            cur_ok = (xb[i + 5] == 3 && xb[i + 6] == 1 && xb[i + 7] == 2);      /* class HID, subclass boot, protocol mouse */
        } else if (type == 5 && len >= 7 && cur_ok && iface < 0 && (xb[i + 2] & 0x80) && (xb[i + 3] & 3) == 3) {
            iface = cur_if; ep_addr = xb[i + 2]; ep_mps = xb[i + 4] | (xb[i + 5] << 8); ep_int = xb[i + 6];
        }
        i += len;
    }
    if (iface < 0) { FAIL(5, 0); log_info("XHCI", "port %u: not a HID boot mouse", p); return; }
    if (hid_slot) return;                                         /* one mouse is enough */
    if (ep_mps < 3 || ep_mps > 64) ep_mps = 8;

    uint32_t dci = (ep_addr & 0xF) * 2 + 1;
    uint32_t ival;
    if (speed >= 3) ival = ep_int ? ep_int - 1 : 0;
    else { uint32_t v = ep_int * 8; ival = 0; while ((2u << ival) <= v) ival++; if (ival < 3) ival = 3; if (ival > 10) ival = 10; }
    if (ival > 15) ival = 15;

    if (ring_init(&hid_ring)) return;
    hid_buf = dma_page(&hid_buf_pa);
    if (!hid_buf) return;

    memset(in, 0, 4096);
    ((uint32_t *)in)[1] = 1u | (1u << dci);                       /* add: slot context + the interrupt endpoint */
    memcpy(in + cs, out, cs);                                     /* current slot context */
    sl = (uint32_t *)(in + cs);
    sl[0] = (sl[0] & ~(0x1Fu << 27)) | (dci << 27);               /* context entries = highest endpoint used */
    uint32_t *ep = (uint32_t *)(in + (1 + dci) * cs);
    uint64_t hdp = hid_ring.pa | 1;
    ep[0] = ival << 16;
    ep[1] = (3u << 1) | (7u << 3) | (ep_mps << 16);               /* error count 3, interrupt IN */
    ep[2] = (uint32_t)hdp; ep[3] = (uint32_t)(hdp >> 32);
    ep[4] = ep_mps | (ep_mps << 16);
    code = command(in_pa, (12u << 10) | (slot << 24), &ev);       /* configure endpoint */
    if (code != 1) { FAIL(6, code); log_info("XHCI", "port %u: configure endpoint failed (%d)", p, code); return; }

    control(slot, ep0, 0x00, 9, cfgval, 0, 0);                    /* SET_CONFIGURATION */
    control(slot, ep0, 0x21, 0x0B, 0, (uint16_t)iface, 0);        /* HID SET_PROTOCOL: boot protocol (3-byte reports) */

    hid_len = ep_mps; hid_errors = 0; hid_slot = (int)slot; hid_dci = (int)dci;
    hid_port = (int)p;
    ring_put(&hid_ring, hid_buf_pa, hid_len, (1u << 10) | (1u << 5) | (1u << 2));
    ring_db(slot, dci);
    log_info("XHCI", "HID mouse %x:%x ready on port %u (slot %u, endpoint %u, interval %u)", vid, pid, p, slot, dci, ival);
}

/* ---- controller start ---- */
static void bios_handoff(uint32_t hcc1)                           /* the firmware may still own the controller: ask for it */
{
    uint32_t off = (hcc1 >> 16) * 4;
    while (off) {
        uint32_t c = R32(cap, off);
        if ((c & 0xFF) == 1) {                                                        /* USB Legacy Support capability */
            if (c & (1u << 16)) {
                W32(cap, off, c | (1u << 24));                                        /* request OS ownership */
                for (int i = 0; i < 1000000 && (R32(cap, off) & (1u << 16)); i++) udelay(1);
            }
            uint32_t s = R32(cap, off + 4);
            s &= ~((1u << 0) | (1u << 4) | (1u << 13) | (1u << 14) | (1u << 15));    /* SMI enables off */
            s |= (7u << 29);                                                          /* clear pending SMI status */
            W32(cap, off + 4, s);
            return;
        }
        uint32_t next = (c >> 8) & 0xFF;
        if (!next) break;
        off += next * 4;
    }
}



void read_protocols(uint32_t hcc1)                        /* the "Supported Protocol" extended capabilities */
{
    uint32_t off = (hcc1 >> 16) * 4;
    while (off) {
        uint32_t c = R32(cap, off);
        if ((c & 0xFF) == 2) {
            uint32_t major = c >> 24, dw2 = R32(cap, off + 8);
            uint32_t first = dw2 & 0xFF, count = (dw2 >> 8) & 0xFF;
            log_info("XHCI", "protocol USB%u: ports %u..%u", major, first, first + count - 1);
            if (major == 2 && !usb2_count) { usb2_first = (uint8_t)first; usb2_count = (uint8_t)count; }
        }
        uint32_t next = (c >> 8) & 0xFF;
        if (!next) break;
        off += next * 4;
    }
}

int is_usb2(uint32_t p)
{
    return !usb2_count || (p >= usb2_first && p < usb2_first + usb2_count);
}



void power_ports(uint32_t n)
{
    for (uint32_t p = 1; p <= n; p++) {
        uint32_t sc = R32(portreg(p), 0);
        if (!(sc & (1u << 9))) W32(portreg(p), 0, (sc & ~PORTSC_RW1C) | (1u << 9));
    }
    msleep(100);
}

int connected_usb2(uint32_t n)
{
    int c = 0;
    for (uint32_t p = 1; p <= n; p++) if (is_usb2(p) && (R32(portreg(p), 0) & 1)) c++;
    return c;
}

void dump_ports(uint32_t n, const char *when)
{
    for (uint32_t p = 1; p <= n; p++) {
        uint32_t sc = R32(portreg(p), 0);
        log_info("XHCI", "%s port %u (%s): PORTSC=0x%x ccs=%u pls=%u speed=%u", when, p,
                 is_usb2(p) ? "usb2" : "usb3", sc, sc & 1, (sc >> 5) & 0xF, (sc >> 10) & 0xF);
    }
}

void ack_changes(uint32_t n)
{
    for (uint32_t p = 1; p <= n; p++) {
        uint32_t sc = R32(portreg(p), 0);
        if (sc & (0x7Fu << 17)) W32(portreg(p), 0, (sc & ~PORTSC_RW1C) | (sc & (0x7Fu << 17)));
    }
}


int xhci_init(void)
{
    tsc_calibrate();
    pci_dev_t d;
    if (pci_find_class(0x0C, 0x03, 0x30, &d)) { log_info("XHCI", "no xHCI controller"); return -1; }

    uint32_t cmd = pci_read32(d.bus, d.dev, d.fn, 0x04);
    pci_write32(d.bus, d.dev, d.fn, 0x04, (cmd & 0xFFFF) | 0x6);                      /* memory space + bus master */
    uint32_t b0 = pci_read32(d.bus, d.dev, d.fn, 0x10);
    uint64_t bar = b0 & ~0xFULL;
    if (((b0 >> 1) & 3) == 2) bar |= (uint64_t)pci_read32(d.bus, d.dev, d.fn, 0x14) << 32;

    for (int i = 0; i < MMIO_PAGES; i++)
        if (map_page(kernel_pml4_virt, MMIO_VA + i * 4096ULL, bar + i * 4096ULL,
                     PAGE_PRESENT | PAGE_RW | (1ULL << 4) | (1ULL << 3)) != 0) {
            log_error("XHCI", "cannot map BAR at 0x%llx", (unsigned long long)bar); return -1;
        }
    cap = (volatile uint8_t *)MMIO_VA;

    uint32_t w0 = R32(cap, 0);                                                        /* 32-bit access only: byte/word reads aren't guaranteed */
    uint32_t caplen = w0 & 0xFF;
    uint32_t hcs1 = R32(cap, 4), hcs2 = R32(cap, 8), hcc1 = R32(cap, 0x10);
    op = cap + caplen;
    rt = cap + (R32(cap, 0x18) & ~0x1Fu);
    db = cap + (R32(cap, 0x14) & ~3u);
    cs = (hcc1 & 4) ? 64 : 32;
    uint32_t nslots = hcs1 & 0xFF, nports = hcs1 >> 24;
    log_info("XHCI", "bar=0x%llx version=0x%x slots=%u ports=%u context=%u", (unsigned long long)bar, w0 >> 16, nslots, nports, cs);

    bios_handoff(hcc1);
    read_protocols(hcc1);

    if (!(R32(op, 4) & 1)) {
        W32(op, 0, R32(op, 0) & ~1u);
        if (wait_for(op, 4, 1, 1, 100)) log_error("XHCI", "does not halt");
    }
    W32(op, 0, R32(op, 0) | 2u);                                                      /* reset */
    if (wait_for(op, 0, 2, 0, 1000) || wait_for(op, 4, 1u << 11, 0, 1000)) { log_error("XHCI", "reset timeout"); return -1; }

    uint64_t pa;
    dcbaa = dma_page(&pa); uint64_t dcbaa_pa = pa;
    uint32_t nsp = (((hcs2 >> 21) & 0x1F) << 5) | ((hcs2 >> 27) & 0x1F);             /* scratchpad pages the controller wants */
    if (!dcbaa || nsp > 512) { log_error("XHCI", "memory/scratchpad (%u)", nsp); return -1; }
    if (nsp) {
        uint64_t *arr = dma_page(&pa); if (!arr) return -1;
        dcbaa[0] = pa;
        for (uint32_t i = 0; i < nsp; i++) { uint64_t sp; if (!dma_page(&sp)) return -1; arr[i] = sp; }
    }
    evr = dma_page(&pa); evr_pa = pa;
    uint64_t *erst = dma_page(&pa); uint64_t erst_pa = pa;
    xb = dma_page(&xb_pa);
    if (!evr || !erst || !xb || ring_init(&cmdr)) return -1;
    erst[0] = evr_pa; erst[1] = 256;                                                  /* one event-ring segment of 256 TRBs */

    W32(op, 0x38, nslots);                                                            /* CONFIG */
    W64(op, 0x30, dcbaa_pa);                                                          /* DCBAAP */
    W64(op, 0x18, cmdr.pa | 1);                                                       /* CRCR: ring base, cycle state 1 */
    volatile uint8_t *ir = rt + 0x20;
    W32(ir, 8, 1);                                                                    /* ERSTSZ */
    W64(ir, 0x18, evr_pa);                                                            /* ERDP */
    W64(ir, 0x10, erst_pa);                                                           /* ERSTBA */
    ev_deq = 0; ev_ccs = 1;

    W32(op, 0, R32(op, 0) | 1u);                                                      /* run */
    if (wait_for(op, 4, 1, 0, 100)) { log_error("XHCI", "does not start"); return -1; }

    trb_t ev;
    int code = command(0, 23u << 10, &ev);                                            /* no-op: proves the command ring works */
    log_info("XHCI", "no-op: %s, completion code %d", code < 0 ? "TIMEOUT" : "ok", code);


    nports_g = nports;

    // TODO: Uncomment this, it is needed for USB mouse work on actual OS, but in QEMU with that UI is not showing
    // power_ports(nports);
    // int waited = 0, found = 0;
    // for (; waited < 3000; waited += 10) {                 /* real controllers need time to report an attached device */
    //     found = connected_usb2(nports);
    //     if (found) break;
    //     msleep(10);
    // }
    // if (found) msleep(150);                                /* USB debounce */
    // log_info("XHCI", "%d USB2 port(s) connected after %d ms", found, waited);
    dump_ports(nports, "scan");

    uint8_t mask = 0;
    for (uint32_t p = 1; p <= nports && p <= 8; p++) if (R32(portreg(p), 0) & 1) mask |= 1u << (p - 1);
    for (uint32_t p = 1; p <= nports; p++) enumerate_port(p);
    ack_changes(nports);
    
    // uint8_t mask = 0;
    // for (uint32_t p = 1; p <= nports; p++) {                                          /* power on, then list */
    //     volatile uint8_t *pr = op + 0x400 + 0x10 * (p - 1);
    //     uint32_t sc = R32(pr, 0);
    //     if (!(sc & (1u << 9))) { W32(pr, 0, (sc & ~PORTSC_RW1C) | (1u << 9)); udelay(50000); sc = R32(pr, 0); }
    //     if ((sc & 1) && p <= 8) mask |= 1u << (p - 1);
    //     log_info("XHCI", "port %u: PORTSC=0x%x %s speed=%u", p, sc, (sc & 1) ? "DEVICE" : "empty", (sc >> 10) & 0xF);
    // }
    // for (uint32_t p = 1; p <= nports; p++) enumerate_port(p);

    xhci_ready = 1;
    // boot_bits(5, mask, 0x0000FF00);                                                   /* green: ports with a device */
    // boot_bits(6, (uint8_t)code, 0x000000FF);                                          /* blue: no-op code, 00000001 = ok */
    // boot_bits(7, (uint8_t)nports, 0x00FFFFFF);                                        /* white: number of ports */
    // boot_bits(8, hid_slot ? 0xFF : 0x00, 0x0000FF00);                                 /* green: a HID mouse is running */
    // boot_bits(9, dbg_ok, 0x00FFFF00);                                                 /* yellow: devices whose descriptor was read */
    // boot_bits(10, dbg_err, 0x00FF0000);                                               /* red: last failure: high nibble = step, low = code */
    return 0;
}
