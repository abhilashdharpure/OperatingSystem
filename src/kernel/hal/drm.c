// drm.c: a minimal DRM/KMS device on the boot framebuffer ("simpledrm")
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <drm/drm_fourcc.h>
#include "drivers/display/display.h"
#include "debug.h"
#include "errno.h"
#include "paging.h"
#include "kmalloc.h"
#include "string.h"
#include "file.h"
#include "vfs.h"
#include "drivers/display/bochs.h"

#ifndef EFAULT
#define EFAULT 14
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EBUSY
#define EBUSY 16
#endif
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOENT
#define ENOENT 2
#endif
#ifndef ENOTTY
#define ENOTTY 25
#endif
#ifndef ENOSPC
#define ENOSPC 28
#endif
#define K_EAGAIN 11

/* ------------------------------------------------------------ */

#define CONN_ID    10
#define ENC_ID     20
#define CRTC_ID    30
#define FB_ID_BASE 100
#define MAX_DUMB   16
#define MAX_FB     16
#define MAP_STRIDE 0x4000000ULL          /* each dumb buffer owns 64 MB of mmap-offset space */
#define VBLANK_MS  16


extern uint32_t fb_mapped_bytes(void);

static int      g_bochs, g_dbl_tried;
static uint32_t g_cur_w, g_cur_h, g_pitch;
static int      g_frames = 1, g_visible = 0;
static uint32_t g_vram;
static struct drm_mode_modeinfo g_modes[8];
static int      g_nmodes;

/* ---- backing memory for dumb buffers ---- */
#define DRM_VA_BASE   0xFFFFFFFFC4000000ULL      /* 64 MB above KERNEL_FB_VA: check nothing else maps here */
#define SLOT_VA_SIZE  (16ULL << 20)              /* largest dumb buffer: 16 MB (1920x1080x4 is 8.3 MB) */
#define DRM_PAGE      4096ULL

extern uint64_t *kernel_pml4_virt;
extern uint64_t pmm_alloc_page(void);            /* ADAPT: physical address, 0 on failure */
extern void     pmm_free_page(uint64_t pa);      /* ADAPT */

static int      g_slot_busy[MAX_DUMB];           /* global: every open file shares the window */
static uint32_t g_slot_pages[MAX_DUMB];          /* pages already mapped in each slot; they are kept for reuse */

static uint8_t *slot_va(int slot) { return (uint8_t *)(DRM_VA_BASE + (uint64_t)slot * SLOT_VA_SIZE); }

static int slot_ensure(int slot, size_t bytes)
{
    uint32_t need = (uint32_t)((bytes + DRM_PAGE - 1) / DRM_PAGE);
    while (g_slot_pages[slot] < need) {
        uint64_t pa = pmm_alloc_page();
        if (!pa) return -ENOMEM;
        uint64_t va = DRM_VA_BASE + (uint64_t)slot * SLOT_VA_SIZE + (uint64_t)g_slot_pages[slot] * DRM_PAGE;
        if (map_page(kernel_pml4_virt, va, pa, PAGE_PRESENT | PAGE_RW) != 0) {
            pmm_free_page(pa);
            return -ENOMEM;
        }
        g_slot_pages[slot]++;
    }
    return 0;
}

static const struct { uint16_t w, h; } k_mode_table[] = {
    { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1280, 720 }, { 1366, 768 },
};   /* keep this list at or below what the client shm pools can hold, see the cautions below */

/* ---------------- ADAPT these to your kernel ---------------- */
static inline uint64_t drm_now_ms(void)   { return get_system_time_ms(); }      // monotonic milliseconds
// Same page loop as your fb_mmap: for each 4 KB page, phys = get_mapped_phys(kva + off), map it into the
// current process (user, read/write, marked shared so process teardown never frees it). Returns 0 or -errno.
extern int map_kernel_range_to_user(const void *kva, uint64_t len, uint64_t prot, uint64_t *out_user_va);
extern uint8_t g_edid[128];
extern int     g_edid_valid;

typedef struct {
    int used, destroyed, fb_refs;
    uint32_t handle, width, height, pitch;
    size_t size;
    void *raw;
    uint8_t *kva;
    uint64_t map_off;
} dumb_t;

typedef struct { int used; uint32_t id, handle, width, height; } drmfb_t;

typedef struct {
    dumb_t dumb[MAX_DUMB];
    drmfb_t fb[MAX_FB];
    uint32_t next_handle, next_fb, crtc_fb, seq;
    int flip_pending;
    uint64_t flip_ready_ms, flip_user_data;
} drm_file_t;

#define UPTR(p) ((void *)(uintptr_t)(p))
static int user_ok(const void *p, size_t n)
{
    return p && (uintptr_t)p >= 0x10000 && (uintptr_t)p + n <= 0x00007FFFFFFFFFFFULL;
}

static int put_ids(uint64_t uptr, uint32_t ucount, const uint32_t *ids, uint32_t n)
{
    if (uptr && ucount >= n && n) {
        if (!user_ok(UPTR(uptr), n * 4)) return -EFAULT;
        memcpy(UPTR(uptr), ids, n * 4);
    }
    return 0;
}

static void fill_mode(struct drm_mode_modeinfo *m)
{
    uint32_t w = fb_dev.width, h = fb_dev.height;
    memset(m, 0, sizeof *m);
    m->hdisplay = w; m->hsync_start = w + 16; m->hsync_end = w + 112; m->htotal = w + 160;
    m->vdisplay = h; m->vsync_start = h + 3;  m->vsync_end = h + 9;   m->vtotal = h + 30;
    m->vrefresh = 60;                                   /* nominal: the boot framebuffer has no real timing */
    m->clock = (uint32_t)((uint64_t)m->htotal * m->vtotal * 60 / 1000);
    m->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
    ksnprintf(m->name, DRM_DISPLAY_MODE_LEN, "%ux%u", w, h);
}

static void fill_mode_wh(struct drm_mode_modeinfo *m, uint32_t w, uint32_t h, int preferred)
{
    memset(m, 0, sizeof *m);
    m->hdisplay = w; m->hsync_start = w + 16; m->hsync_end = w + 112; m->htotal = w + 160;
    m->vdisplay = h; m->vsync_start = h + 3;  m->vsync_end = h + 9;   m->vtotal = h + 30;
    m->vrefresh = 60;
    m->clock = (uint32_t)((uint64_t)m->htotal * m->vtotal * 60 / 1000);
    m->type = DRM_MODE_TYPE_DRIVER | (preferred ? DRM_MODE_TYPE_PREFERRED : 0);
    ksnprintf(m->name, DRM_DISPLAY_MODE_LEN, "%dx%d", (int)w, (int)h);
}

static void build_modes(void)
{
    g_nmodes = 0;
    fill_mode_wh(&g_modes[g_nmodes++], g_cur_w, g_cur_h, 1);        /* the boot mode first */
    if (!g_bochs) return;
    for (unsigned i = 0; i < sizeof k_mode_table / sizeof k_mode_table[0] && g_nmodes < 8; i++) {
        uint32_t w = k_mode_table[i].w, h = k_mode_table[i].h;
        if ((w == g_cur_w && h == g_cur_h) || (uint64_t)w * 4 * h > g_vram) continue;
        fill_mode_wh(&g_modes[g_nmodes++], w, h, 0);
    }
}

static int mode_known(uint32_t w, uint32_t h)
{
    for (int i = 0; i < g_nmodes; i++) if (g_modes[i].hdisplay == w && g_modes[i].vdisplay == h) return 1;
    return 0;
}

static int apply_mode(uint16_t w, uint16_t h)
{
    uint32_t pitch = (uint32_t)w * 4;
    int want2 = (uint64_t)pitch * h * 2 <= g_vram;
    if (bochs_set_mode(w, h, (uint16_t)(want2 ? h * 2 : h)) != 0) return -1;
    g_frames = (bochs_virt_height() >= 2u * h) ? 2 : 1;
    g_cur_w = w; g_cur_h = h; g_pitch = pitch; g_visible = 0;
    fb_dev.width = w; fb_dev.height = h; fb_dev.pitch = pitch;      /* keep the kernel console consistent */
    log_info("DRM", "mode %ux%u, %d frame(s) in VRAM", w, h, g_frames);
    return 0;
}

/* ---- buffers ---- */
static dumb_t *dumb_by_handle(drm_file_t *d, uint32_t h)
{
    for (int i = 0; i < MAX_DUMB; i++)
        if (d->dumb[i].used && d->dumb[i].handle == h) return &d->dumb[i];
    return NULL;
}
static drmfb_t *fb_by_id(drm_file_t *d, uint32_t id)
{
    for (int i = 0; i < MAX_FB; i++)
        if (d->fb[i].used && d->fb[i].id == id) return &d->fb[i];
    return NULL;
}

static void dumb_free(dumb_t *b)
{
    if (b->used) g_slot_busy[b->size] = 0;               /* the pages stay mapped and are reused */
    memset(b, 0, sizeof *b);
}

static uint8_t *frame_base(int frame) { return (uint8_t *)KERNEL_FB_VA + (size_t)frame * g_cur_h * g_pitch; }

static void scanout_blit(const dumb_t *b, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, int frame)
{
    if (x2 > g_cur_w) x2 = g_cur_w;
    if (x2 > b->width) x2 = b->width;
    if (y2 > g_cur_h) y2 = g_cur_h;
    if (y2 > b->height) y2 = b->height;
    if (x1 >= x2 || y1 >= y2) return;
    uint8_t *dst = frame_base(frame);
    for (uint32_t y = y1; y < y2; y++)
        memcpy(dst + (size_t)y * g_pitch + (size_t)x1 * 4,
               b->kva + (size_t)y * b->pitch + (size_t)x1 * 4, (size_t)(x2 - x1) * 4);
}
/* ---- events ---- */
static int event_ready(drm_file_t *d) { return d->flip_pending && drm_now_ms() >= d->flip_ready_ms; }
static int drm_can_read(struct file *f) { return event_ready((drm_file_t *)f->private_data); }

static int drm_read(struct file *f, void *buf, size_t size)
{
    drm_file_t *d = (drm_file_t *)f->private_data;
    if (!event_ready(d)) return -K_EAGAIN;
    if (size < sizeof(struct drm_event_vblank)) return -EINVAL;
    struct drm_event_vblank ev;
    memset(&ev, 0, sizeof ev);
    uint64_t ms = drm_now_ms();
    ev.base.type = DRM_EVENT_FLIP_COMPLETE;
    ev.base.length = sizeof ev;
    ev.user_data = d->flip_user_data;
    ev.tv_sec = (uint32_t)(ms / 1000);
    ev.tv_usec = (uint32_t)((ms % 1000) * 1000);
    ev.sequence = ++d->seq;
    ev.crtc_id = CRTC_ID;
    memcpy(buf, &ev, sizeof ev);
    d->flip_pending = 0;
    return (int)sizeof ev;
}

/* ---- ioctl ---- */
static int drm_ioctl(struct file *f, unsigned long req_in, void *arg)
{
    unsigned int req = (unsigned int)req_in;        /* musl sign-extends int commands */
    drm_file_t *d = (drm_file_t *)f->private_data;
    if (_IOC_TYPE(req) != DRM_IOCTL_BASE) return -ENOTTY;
    if (_IOC_SIZE(req) && !user_ok(arg, _IOC_SIZE(req))) return -EFAULT;

    switch (req) {
    case DRM_IOCTL_SET_MASTER:
    case DRM_IOCTL_DROP_MASTER:
        return 0;

    case DRM_IOCTL_GET_CAP: {
        struct drm_get_cap *c = arg;
        switch (c->capability) {
        case DRM_CAP_DUMB_BUFFER:          c->value = 1; return 0;
        case DRM_CAP_DUMB_PREFERRED_DEPTH: c->value = 24; return 0;
        case DRM_CAP_DUMB_PREFER_SHADOW:   c->value = 0; return 0;
        case DRM_CAP_TIMESTAMP_MONOTONIC:  c->value = 1; return 0;
        default:                           c->value = 0; return -EINVAL;
        }
    }

    case DRM_IOCTL_SET_CLIENT_CAP:           /* no planes or atomic: clients fall back to the legacy API */
        return -EINVAL;

    case DRM_IOCTL_MODE_GETRESOURCES: {
        struct drm_mode_card_res *r = arg;
        uint32_t fbs[MAX_FB], nfb = 0;
        for (int i = 0; i < MAX_FB; i++) if (d->fb[i].used) fbs[nfb++] = d->fb[i].id;
        uint32_t crtc = CRTC_ID, enc = ENC_ID, conn = CONN_ID;
        int e;
        if ((e = put_ids(r->fb_id_ptr, r->count_fbs, fbs, nfb))) return e;
        if ((e = put_ids(r->crtc_id_ptr, r->count_crtcs, &crtc, 1))) return e;
        if ((e = put_ids(r->connector_id_ptr, r->count_connectors, &conn, 1))) return e;
        if ((e = put_ids(r->encoder_id_ptr, r->count_encoders, &enc, 1))) return e;
        r->count_fbs = nfb; r->count_crtcs = 1; r->count_connectors = 1; r->count_encoders = 1;
        r->min_width = 64;
        r->max_width = g_cur_w;
        r->min_height = 64;
        r->max_height = g_cur_h;
        return 0;
    }

    case DRM_IOCTL_MODE_GETENCODER: {
        struct drm_mode_get_encoder *e = arg;
        if (e->encoder_id != ENC_ID) return -ENOENT;
        e->encoder_type = DRM_MODE_ENCODER_VIRTUAL;
        e->crtc_id = CRTC_ID;
        e->possible_crtcs = 1;
        e->possible_clones = 0;
        return 0;
    }

    case DRM_IOCTL_MODE_GETCONNECTOR: {
        struct drm_mode_get_connector *c = arg;
        if (c->connector_id != CONN_ID) return -ENOENT;
        fill_mode(g_modes);

        uint32_t enc = ENC_ID;

        if (c->modes_ptr && c->count_modes >= (uint32_t)g_nmodes) {
            if (!user_ok(UPTR(c->modes_ptr), g_nmodes * sizeof(struct drm_mode_modeinfo))) return -EFAULT;
            memcpy(UPTR(c->modes_ptr), g_modes, g_nmodes * sizeof(struct drm_mode_modeinfo));
        }

        int e = put_ids(c->encoders_ptr, c->count_encoders, &enc, 1);
        if (e) return e;
        c->count_modes = g_nmodes;
        c->count_props = 0; c->count_encoders = 1;
        c->encoder_id = ENC_ID;
        c->connector_type = DRM_MODE_CONNECTOR_VIRTUAL;
        c->connector_type_id = 1;
        c->connection = DRM_MODE_CONNECTED;
        c->mm_width  = g_edid_valid ? g_edid[21] * 10u : 0;      /* 0 = unknown */
        c->mm_height = g_edid_valid ? g_edid[22] * 10u : 0;
        c->subpixel = 0;
        return 0;
    }

    case DRM_IOCTL_MODE_GETCRTC: {
        struct drm_mode_crtc *c = arg;
        if (c->crtc_id != CRTC_ID) return -ENOENT;
        c->fb_id = d->crtc_fb; c->x = 0; c->y = 0; c->gamma_size = 0;
        c->mode_valid = d->crtc_fb != 0;
        if (c->mode_valid) fill_mode_wh(&c->mode, g_cur_w, g_cur_h, 1); else memset(&c->mode, 0, sizeof c->mode);
        return 0;
    }

    case DRM_IOCTL_MODE_SETCRTC: {
        struct drm_mode_crtc *c = arg;
        if (c->crtc_id != CRTC_ID) return -ENOENT;
        if (c->fb_id == 0) { d->crtc_fb = 0; return 0; }          /* disable */
        drmfb_t *fb = fb_by_id(d, c->fb_id);
        if (!fb) return -ENOENT;
        if (!c->mode_valid || c->x || c->y) return -EINVAL;
        uint32_t w = c->mode.hdisplay, h = c->mode.vdisplay;
        if (!mode_known(w, h)) return -EINVAL;
        if (g_bochs && (w != g_cur_w || h != g_cur_h || (!g_dbl_tried && g_frames == 1))) {
            g_dbl_tried = 1;                                  /* re-applying the boot mode enables double buffering */
            if (apply_mode((uint16_t)w, (uint16_t)h) != 0) return -EIO;
        } else if (w != g_cur_w || h != g_cur_h) return -EINVAL;      /* no hardware to switch with */
        if (fb->width != g_cur_w || fb->height != g_cur_h) return -EINVAL;
        dumb_t *b = dumb_by_handle(d, fb->handle);
        if (!b) return -ENOENT;
        scanout_blit(b, 0, 0, g_cur_w, g_cur_h, g_visible);
        d->crtc_fb = fb->id;
        return 0;
    }

    case DRM_IOCTL_MODE_CREATE_DUMB: {
        struct drm_mode_create_dumb *c = arg;
        if (c->bpp != 32 || !c->width || !c->height || c->width > 8192 || c->height > 8192) return -EINVAL;
        uint32_t pitch = c->width * 4;
        uint64_t size = (uint64_t)pitch * c->height;
        if (size > SLOT_VA_SIZE) return -EINVAL;
        int slot = -1;
        for (int i = 0; i < MAX_DUMB; i++) if (!d->dumb[i].used) { slot = i; break; }
        int gs = -1;
        for (int i = 0; i < MAX_DUMB; i++) if (!g_slot_busy[i]) { gs = i; break; }
        if (slot < 0 || gs < 0) return -ENOSPC;

        int e = slot_ensure(gs, (size_t)size);
        if (e) { log_error("DRM", "dumb buffer %ux%u: out of memory", c->width, c->height); return e; }

        g_slot_busy[gs] = 1;
        dumb_t *b = &d->dumb[slot];
        memset(b, 0, sizeof *b);
        b->used = 1; b->size = gs;
        b->kva = slot_va(gs);
        memset(b->kva, 0, (size_t)size);
        b->width = c->width; b->height = c->height; b->pitch = pitch; b->size = (size_t)size;
        b->handle = ++d->next_handle;
        b->map_off = (uint64_t)(gs + 1) * MAP_STRIDE;       /* unique across all open files */
        c->handle = b->handle; c->pitch = pitch; c->size = size;
        return 0;
    }

    case DRM_IOCTL_MODE_MAP_DUMB: {
        struct drm_mode_map_dumb *m = arg;
        dumb_t *b = dumb_by_handle(d, m->handle);
        if (!b || b->destroyed) return -ENOENT;
        m->offset = b->map_off;
        return 0;
    }

    case DRM_IOCTL_MODE_DESTROY_DUMB: {
        struct drm_mode_destroy_dumb *x = arg;
        dumb_t *b = dumb_by_handle(d, x->handle);
        if (!b) return -ENOENT;
        b->destroyed = 1;
        if (b->fb_refs == 0) dumb_free(b);      /* the compositor must munmap before this */
        return 0;
    }

    case DRM_IOCTL_MODE_ADDFB:
    case DRM_IOCTL_MODE_ADDFB2: {
        uint32_t handle, w, h;
        uint32_t *out_id;
        if (req == DRM_IOCTL_MODE_ADDFB) {
            struct drm_mode_fb_cmd *c = arg;
            if (c->bpp != 32 || (c->depth != 24 && c->depth != 32)) return -EINVAL;
            handle = c->handle; w = c->width; h = c->height; out_id = &c->fb_id;
        } else {
            struct drm_mode_fb_cmd2 *c = arg;
            if (c->pixel_format != DRM_FORMAT_XRGB8888 && c->pixel_format != DRM_FORMAT_ARGB8888) return -EINVAL;
            if (c->offsets[0]) return -EINVAL;
            handle = c->handles[0]; w = c->width; h = c->height; out_id = &c->fb_id;
        }
        dumb_t *b = dumb_by_handle(d, handle);
        if (!b || b->destroyed || w > b->width || h > b->height) return -EINVAL;
        for (int i = 0; i < MAX_FB; i++) {
            if (d->fb[i].used) continue;
            d->fb[i].used = 1; d->fb[i].id = d->next_fb ? d->next_fb++ : (d->next_fb = FB_ID_BASE, d->next_fb++);
            d->fb[i].handle = handle; d->fb[i].width = w; d->fb[i].height = h;
            b->fb_refs++;
            *out_id = d->fb[i].id;
            return 0;
        }
        return -ENOSPC;
    }

    case DRM_IOCTL_MODE_RMFB: {
        uint32_t id = *(uint32_t *)arg;
        drmfb_t *fb = fb_by_id(d, id);
        if (!fb) return -ENOENT;
        dumb_t *b = dumb_by_handle(d, fb->handle);
        if (d->crtc_fb == id) d->crtc_fb = 0;
        memset(fb, 0, sizeof *fb);
        if (b && --b->fb_refs <= 0 && b->destroyed) dumb_free(b);
        return 0;
    }

    case DRM_IOCTL_MODE_PAGE_FLIP: {
        struct drm_mode_crtc_page_flip *p = arg;
        if (p->crtc_id != CRTC_ID) return -ENOENT;
        if (!d->crtc_fb) return -EINVAL;                           /* SETCRTC first */
        if (p->flags & ~DRM_MODE_PAGE_FLIP_EVENT) return -EINVAL;
        if (d->flip_pending) return -EBUSY;
        drmfb_t *fb = fb_by_id(d, p->fb_id);
        if (!fb) return -ENOENT;
        dumb_t *b = dumb_by_handle(d, fb->handle);
        if (!b) return -ENOENT;

        int hidden = (g_frames == 2) ? 1 - g_visible : 0;
        scanout_blit(b, 0, 0, g_cur_w, g_cur_h, hidden);
        if (g_frames == 2) { bochs_set_y((uint16_t)(hidden * g_cur_h)); g_visible = hidden; }
        d->crtc_fb = fb->id;

        if (p->flags & DRM_MODE_PAGE_FLIP_EVENT) {
            d->flip_pending = 1;
            d->flip_user_data = p->user_data;
            d->flip_ready_ms = drm_now_ms() + VBLANK_MS;
        }
        return 0;
    }

    case DRM_IOCTL_MODE_DIRTYFB: {
        struct drm_mode_fb_dirty_cmd *c = arg;
        drmfb_t *fb = fb_by_id(d, c->fb_id);
        if (!fb) return -ENOENT;
        if (c->fb_id != d->crtc_fb) return 0;                      /* not on screen */
        dumb_t *b = dumb_by_handle(d, fb->handle);

        if (!b) return -ENOENT;
        if (!c->num_clips || !c->clips_ptr) {
            scanout_blit(b, 0, 0, b->width, b->height, g_visible);
        } else {
            uint32_t n = c->num_clips > 64 ? 64 : c->num_clips;
            if (!user_ok(UPTR(c->clips_ptr), n * sizeof(struct drm_clip_rect))) return -EFAULT;
            const struct drm_clip_rect *cr = UPTR(c->clips_ptr);
            for (uint32_t i = 0; i < n; i++) scanout_blit(b, cr[i].x1, cr[i].y1, cr[i].x2, cr[i].y2, g_visible);
        }
        return 0;
    }

    default:
        log_info("DRM", "unhandled ioctl 0x%lx (nr 0x%x)", req, (unsigned)_IOC_NR(req));
        return -ENOTTY;
    }
}

static int drm_mmap(struct file *f, uint64_t length, uint64_t prot, uint64_t flags,
                    uint64_t offset, uint64_t *out_user_va)
{
    log_info("DRM", "mmap off=0x%llx len=%llu", (unsigned long long)offset, (unsigned long long)length);
    (void)flags;
    drm_file_t *d = (drm_file_t *)f->private_data;
    for (int i = 0; i < MAX_DUMB; i++) {
        dumb_t *b = &d->dumb[i];
        if (!b->used || b->destroyed || b->map_off != offset) continue;
        uint64_t max = ((uint64_t)b->size + 4095) & ~4095ULL;
        if (length > max) return -EINVAL;
        return map_kernel_range_to_user(b->kva, length, prot, out_user_va);
    }
    return -EINVAL;
}

static int drm_open(struct file *f, int flags)
{
    (void)flags;
    drm_file_t *d = (drm_file_t *)kmalloc(sizeof *d);
    if (!d) return -ENOMEM;
    memset(d, 0, sizeof *d);
    d->next_fb = FB_ID_BASE;
    f->private_data = d;
    return 0;
}

static int drm_close(struct file *f)
{
    drm_file_t *d = (drm_file_t *)f->private_data;
    if (d) {
        for (int i = 0; i < MAX_DUMB; i++) if (d->dumb[i].used) dumb_free(&d->dumb[i]);
        kfree(d);
        f->private_data = NULL;
    }
    return 0;
}

static struct file_operations drm_fops = {
    .open    = drm_open,
    .close   = drm_close,
    .read    = drm_read,
    .ioctl   = drm_ioctl,
    .can_read = drm_can_read,
    .mmap    = drm_mmap,
};

void drm_init(void)
{
    if (fb_dev.bpp != 32) { log_error("DRM", "framebuffer is %u bpp, only 32 is supported", (unsigned)fb_dev.bpp); return; }

    g_cur_w = fb_dev.width; g_cur_h = fb_dev.height; g_pitch = fb_dev.pitch;
    g_bochs = bochs_present();
    g_vram  = g_bochs ? fb_mapped_bytes() : (uint32_t)(g_pitch * g_cur_h);
    build_modes();

    VFS_RegisterDevice("/dev/dri/card0", &drm_fops, NULL);
    log_info("DRM", "/dev/dri/card0 registered (%ux%u, simple framebuffer)", fb_dev.width, fb_dev.height);
}