#include "drm_output.h"
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <iostream>

DrmOutput g_drm;

static int xioctl(int fd, unsigned long req, void* arg)
{
    int r;
    do { r = ioctl(fd, req, arg); } while (r < 0 && errno == EINTR);
    return r;
}
static uint64_t ptr(const void* p) { return (uint64_t)(uintptr_t)p; }
static int64_t now_ms()
{
    timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool DrmOutput::fail(const char* what)
{
    std::printf("[drm] %s (errno %d)\n", what, errno);
    if (fd_ >= 0) close(fd_);
    fd_ = -1;
    return false;
}

bool DrmOutput::make_buf(Buf& b)
{
    drm_mode_create_dumb cd{};
    cd.width = w_; cd.height = h_; cd.bpp = 32;
    if (xioctl(fd_, DRM_IOCTL_MODE_CREATE_DUMB, &cd) != 0) 
    {
        std::cout<<" DrmOutput::make_buf return if (xioctl(fd_, DRM_IOCTL_MODE_CREATE_DUMB, &cd) != 0) "<<std::endl;
        return false;
    }

    drm_mode_map_dumb md{};
    md.handle = cd.handle;
    if (xioctl(fd_, DRM_IOCTL_MODE_MAP_DUMB, &md) != 0) return false;

    void* p = mmap(nullptr, cd.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, (off_t)md.offset);
    if (p == MAP_FAILED)
    {
        std::cout<<" DrmOutput::make_buf return if (p == MAP_FAILED)"<<std::endl;
        return false;
    }

    drm_mode_fb_cmd fb{};
    fb.width = w_; fb.height = h_; fb.pitch = cd.pitch; fb.bpp = 32; fb.depth = 24; fb.handle = cd.handle;
    if (xioctl(fd_, DRM_IOCTL_MODE_ADDFB, &fb) != 0)
    {
        std::cout<<" DrmOutput::make_buf return if (xioctl(fd_, DRM_IOCTL_MODE_ADDFB, &fb) != 0)"<<std::endl;
        return false;
    }

    b.handle = cd.handle; b.fb = fb.fb_id; b.pitch = cd.pitch; b.size = cd.size; b.map = (uint8_t*)p;
    return true;
}

bool DrmOutput::open_device(const char* path, int expect_w, int expect_h,
                            const uint32_t* initial, int initial_stride_px)
{
    fd_ = ::open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);

    std::cout<<"DrmOutput::open_device "<<path<<", fd = "<<fd_<<std::endl;

    if (fd_ < 0)
    {
        std::cout<<"DrmOutput::open_device  fd not available: caller falls back to /dev/fb0"<<std::endl;
        return false;                               // not available: caller falls back to /dev/fb0
    }

    drm_get_cap cap{};
    cap.capability = DRM_CAP_DUMB_BUFFER;
    if (xioctl(fd_, DRM_IOCTL_GET_CAP, &cap) != 0 || !cap.value)
    {
        std::cout<<"DrmOutput::open_device return no dumb buffer support "<<std::endl;
        return fail("no dumb buffer support");
    }
    xioctl(fd_, DRM_IOCTL_SET_MASTER, nullptr);

    drm_mode_card_res res{};
    if (xioctl(fd_, DRM_IOCTL_MODE_GETRESOURCES, &res) != 0)
    {
        std::cout<<"DrmOutput::open_device return GETRESOURCES"<<std::endl;

        return fail("GETRESOURCES");
    }
    std::vector<uint32_t> crtcs(res.count_crtcs), conns(res.count_connectors);
    std::vector<uint32_t> encs(res.count_encoders), fbs(res.count_fbs);
    res.crtc_id_ptr = ptr(crtcs.data());      res.connector_id_ptr = ptr(conns.data());
    res.encoder_id_ptr = ptr(encs.data());    res.fb_id_ptr = ptr(fbs.data());
    if (xioctl(fd_, DRM_IOCTL_MODE_GETRESOURCES, &res) != 0)
    {
        std::cout<<"DrmOutput::open_device return GETRESOURCES (ids)"<<std::endl;
        return fail("GETRESOURCES (ids)");
    }

    drm_mode_modeinfo mode{};
    uint32_t enc_id = 0;
    bool found = false;
    for (uint32_t id : conns) {
        drm_mode_get_connector c{};
        c.connector_id = id;
        if (xioctl(fd_, DRM_IOCTL_MODE_GETCONNECTOR, &c) != 0) continue;
        std::vector<drm_mode_modeinfo> modes(c.count_modes);
        std::vector<uint32_t> cencs(c.count_encoders);
        c.modes_ptr = ptr(modes.data());
        c.encoders_ptr = ptr(cencs.data());
        c.count_props = 0; c.props_ptr = 0; c.prop_values_ptr = 0;
        if (xioctl(fd_, DRM_IOCTL_MODE_GETCONNECTOR, &c) != 0) continue;
        if (c.connection != DRM_MODE_CONNECTED || modes.empty()) continue;

        modes_ = modes;
        mode = modes[0];
        for (auto& m : modes) if (m.type & DRM_MODE_TYPE_PREFERRED) { mode = m; break; }
        conn_ = id; enc_id = c.encoder_id; found = true;
        std::printf("[drm] connector %u: %s, %u x %u mm\n", id, mode.name, c.mm_width, c.mm_height);
        break;
    }
    if (!found)
    {
        std::cout<<"DrmOutput::open_device return no connected connector"<<std::endl;
        return fail("no connected connector");
    }

    crtc_ = crtcs.empty() ? 0 : crtcs[0];
    if (enc_id) {
        drm_mode_get_encoder e{};
        e.encoder_id = enc_id;
        if (xioctl(fd_, DRM_IOCTL_MODE_GETENCODER, &e) == 0 && e.crtc_id) crtc_ = e.crtc_id;
    }
    if (!crtc_)
    {
        std::cout<<"DrmOutput::open_device return no crtc"<<std::endl;
        return fail("no crtc");
    }

    w_ = mode.hdisplay; h_ = mode.vdisplay;
    if (expect_w && (w_ != expect_w || h_ != expect_h))
    {
        std::cout<<"DrmOutput::open_device return mode differs from the framebuffer size"<<std::endl;

        return fail("mode differs from the framebuffer size");
    }

    if (!make_buf(buf_[0]) || !make_buf(buf_[1]))
    {
        std::cout<<"DrmOutput::open_device return cannot create buffers"<<std::endl;

        return fail("cannot create buffers");
    }

    if (initial)                                              // keep the boot screen instead of flashing black
        for (int y = 0; y < h_; y++)
            std::memcpy(buf_[0].map + (size_t)y * buf_[0].pitch, initial + (size_t)y * initial_stride_px, (size_t)w_ * 4);

    drm_mode_crtc sc{};
    sc.crtc_id = crtc_; sc.fb_id = buf_[0].fb;
    sc.set_connectors_ptr = ptr(&conn_); sc.count_connectors = 1;
    sc.mode_valid = 1; sc.mode = mode;
    if (xioctl(fd_, DRM_IOCTL_MODE_SETCRTC, &sc) != 0)
    {
        std::cout<<"DrmOutput::open_device return SETCRTC"<<std::endl;

        return fail("SETCRTC");
    }

    front_ = 0;

    std::cout<<"DrmOutput::open_device return true"<<std::endl;

    return true;
}

bool DrmOutput::present(const uint32_t* src, int stride_px)
{
    if (pending_ && now_ms() - pending_since_ms_ > 200) {     // the event never came: don't stall forever
        std::printf("[drm] flip event timeout\n");
        pending_ = false;
    }
    if (pending_) return false;

    int back = 1 - front_;
    Buf& b = buf_[back];
    for (int y = 0; y < h_; y++)
        std::memcpy(b.map + (size_t)y * b.pitch, src + (size_t)y * stride_px, (size_t)w_ * 4);

    drm_mode_crtc_page_flip pf{};
    pf.crtc_id = crtc_; pf.fb_id = b.fb; pf.flags = DRM_MODE_PAGE_FLIP_EVENT; pf.user_data = 1;
    if (xioctl(fd_, DRM_IOCTL_MODE_PAGE_FLIP, &pf) != 0) {
        if (errno != EBUSY) std::printf("[drm] page flip failed (errno %d)\n", errno);
        return false;
    }
    front_ = back;
    pending_ = true;
    pending_since_ms_ = now_ms();
    return true;
}

void DrmOutput::drain_events()
{
    char buf[256];
    for (;;) {
        ssize_t n = read(fd_, buf, sizeof buf);
        if (n <= 0) break;
        for (ssize_t off = 0; off + (ssize_t)sizeof(drm_event) <= n;) {
            auto* e = (drm_event*)(buf + off);
            if (e->length == 0) break;
            if (e->type == DRM_EVENT_FLIP_COMPLETE) pending_ = false;
            off += e->length;
        }
    }
}

void DrmOutput::dirty(int x1, int y1, int x2, int y2)
{
    drm_clip_rect r;
    r.x1 = (uint16_t)std::max(0, x1);     r.y1 = (uint16_t)std::max(0, y1);
    r.x2 = (uint16_t)std::min(w_, x2);    r.y2 = (uint16_t)std::min(h_, y2);
    drm_mode_fb_dirty_cmd c{};
    c.fb_id = buf_[front_].fb; c.num_clips = 1; c.clips_ptr = ptr(&r);
    xioctl(fd_, DRM_IOCTL_MODE_DIRTYFB, &c);
}

void DrmOutput::release(Buf& b)
{
    if (b.map) munmap(b.map, b.size);                         // the kernel's munmap must not free these pages
    if (b.fb)     { uint32_t id = b.fb; xioctl(fd_, DRM_IOCTL_MODE_RMFB, &id); }
    if (b.handle) { drm_mode_destroy_dumb d{}; d.handle = b.handle; xioctl(fd_, DRM_IOCTL_MODE_DESTROY_DUMB, &d); }
    b = Buf{};
}

std::vector<std::pair<int, int>> DrmOutput::mode_sizes() const
{
    std::vector<std::pair<int, int>> v;
    for (auto& m : modes_) v.push_back({m.hdisplay, m.vdisplay});
    return v;
}

bool DrmOutput::set_mode(int w, int h)
{
    if (fd_ < 0) return false;
    if (w == w_ && h == h_) return true;
    const drm_mode_modeinfo* mi = nullptr;
    for (auto& m : modes_) if (m.hdisplay == w && m.vdisplay == h) { mi = &m; break; }
    if (!mi) return false;

    drain_events();
    release(buf_[0]); release(buf_[1]);
    int ow = w_, oh = h_;
    w_ = w; h_ = h;
    bool ok = make_buf(buf_[0]) && make_buf(buf_[1]);
    if (ok) {
        drm_mode_crtc sc{};
        sc.crtc_id = crtc_; sc.fb_id = buf_[0].fb;
        sc.set_connectors_ptr = ptr(&conn_); sc.count_connectors = 1;
        sc.mode_valid = 1; sc.mode = *mi;
        ok = xioctl(fd_, DRM_IOCTL_MODE_SETCRTC, &sc) == 0;
    }
    if (!ok) { std::printf("[drm] mode switch to %dx%d failed\n", w, h); w_ = ow; h_ = oh; return false; }
    front_ = 0; pending_ = false;
    return true;
}