#pragma once
#include "luma-shell-client-protocol.h"
#include <cstdint>
#include <functional>
#include <map>
#include <string>

struct ShellWindow {
    std::string app_id, title;
    bool minimized = false, focused = false;
};

struct ShellClient {
    luma_shell* shell = nullptr;
    std::map<uint32_t, ShellWindow> windows;       // ordered by id: the last entry is the newest
    std::function<void()> on_change;

    static ShellClient& get() { static ShellClient s; return s; }

    void bind(wl_registry* reg, uint32_t name)
    {
        static const luma_shell_listener l = {
            /*window_added*/ [](void*, luma_shell*, uint32_t id, const char* app, const char* title) {
                auto& s = get();
                ShellWindow w; w.app_id = app; w.title = title;
                s.windows[id] = w;
                if (s.on_change) s.on_change(); },
            /*window_state*/ [](void*, luma_shell*, uint32_t id, uint32_t min, uint32_t foc) {
                auto& s = get();
                auto it = s.windows.find(id);
                if (it == s.windows.end()) return;
                it->second.minimized = min != 0;
                it->second.focused   = foc != 0;
                if (s.on_change) s.on_change(); },
            /*window_removed*/ [](void*, luma_shell*, uint32_t id) {
                auto& s = get();
                s.windows.erase(id);
                if (s.on_change) s.on_change(); },
        };
        shell = (luma_shell*)wl_registry_bind(reg, name, &luma_shell_interface, 1);
        luma_shell_add_listener(shell, &l, nullptr);
    }

    void activate(uint32_t id) { if (shell) luma_shell_activate(shell, id); }
    void minimize(uint32_t id) { if (shell) luma_shell_minimize(shell, id); }
    void close_window(uint32_t id) { if (shell) luma_shell_close(shell, id); }
};