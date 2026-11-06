#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>
#include <drm/drm_mode.h>

class DrmOutput {
public:
    // expect_w/h: 0 = accept the connector's preferred mode
    bool open_device(const char* path, int expect_w, int expect_h,
                     const uint32_t* initial, int initial_stride_px);

    bool ready() const { return fd_ >= 0; }
    int  fd() const { return fd_; }
    int  width() const { return w_; }
    int  height() const { return h_; }

    // Copy a full XRGB8888 frame to the hidden buffer and flip. Returns false when a flip is still pending.
    bool present(const uint32_t* src, int stride_px);
    void drain_events();                      // call when fd() is readable

    // The buffer that is currently on screen (for the software cursor)
    uint8_t* front_map() { return buf_[front_].map; }
    size_t   front_pitch() const { return buf_[front_].pitch; }
    void     dirty(int x1, int y1, int x2, int y2);
    bool set_mode(int w, int h);
    std::vector<std::pair<int, int>> mode_sizes() const;

private:

    struct Buf { uint32_t handle = 0, fb = 0, pitch = 0; size_t size = 0; uint8_t* map = nullptr; };
    bool make_buf(Buf& b);
    bool fail(const char* what);

    int fd_ = -1, w_ = 0, h_ = 0;
    uint32_t crtc_ = 0, conn_ = 0;
    Buf buf_[2];
    int front_ = 0;
    bool pending_ = false;
    int64_t pending_since_ms_ = 0;
    void release(Buf& b);
    std::vector<drm_mode_modeinfo> modes_;
};

extern DrmOutput g_drm;