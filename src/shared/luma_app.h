#pragma once
#include "luma_platform.h"
#include <memory>

inline bool luma_start(const char* app_id, uint32_t w = 900, uint32_t h = 500, bool fill_output = false)
{
    g_app_id = app_id;  g_app_title = app_id;
    g_app_w = w;        g_app_h = h;
    g_fill_output = fill_output;
    auto p = std::make_unique<LumaPlatform>();
    if (!p->connect()) return false;
    slint::platform::set_platform(std::move(p));
    return true;
}