// // home_page.h
// #pragma once
// #include "app.h"
// #include <string>

// class HomePage {
// public:
//     explicit HomePage(slint::ComponentHandle<AppWindow> ui);
//     void set_user(const std::string& name);       // C++ -> UI
//     std::string user() const;                      // UI -> C++
// private:
//     long spawn_path(const char* path);

//     slint::ComponentWeakHandle<AppWindow> ui_;
//     int clicks_ = 0;
// };

#pragma once
#include "app.h"
#include <array>
#include <functional>
#include <string>

class HomePage {
public:
    explicit HomePage(slint::ComponentHandle<AppWindow> ui);
    void set_user(const std::string& name);       // C++ -> UI
    std::string user() const;                      // UI -> C++

private:
    struct AppSlot {
        std::string path;
        long        pid = 0;                       // 0 = not running
        std::function<void(bool)> set_open;        // updates the highlight in the UI
    };

    void launch(int idx);
    void poll_children();

    slint::ComponentWeakHandle<AppWindow> ui_;
    std::array<AppSlot, 4>                apps_;
    slint::Timer                          reaper_;  // must stay alive as long as the page
};