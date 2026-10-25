#pragma once
#include "luma-settings-client-protocol.h"
#include <cstdio>
#include <functional>
#include <map>
#include <string>

struct SettingsClient {
    luma_settings* settings = nullptr;
    std::map<std::string, std::string> cache;                       // everything received so far
    std::function<void(const std::string&, const std::string&)> on_changed;

    static SettingsClient& get() { static SettingsClient s; return s; }

    void bind(wl_registry* reg, uint32_t name)
    {
        static const luma_settings_listener l = {
            /*changed*/ [](void*, luma_settings*, const char* k, const char* v) {
                auto& s = get();
                s.cache[k] = v;
                if (s.on_changed) s.on_changed(k, v);
            }
        };
        settings = (luma_settings*)wl_registry_bind(reg, name, &luma_settings_interface, 1);
        luma_settings_add_listener(settings, &l, nullptr);
    }

    void set_settings(const std::string& key, const std::string& value)
    {
        cache[key] = value;
        if (settings) luma_settings_set(settings, key.c_str(), value.c_str());
    }
};