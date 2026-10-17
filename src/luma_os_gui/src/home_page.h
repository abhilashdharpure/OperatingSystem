// home_page.h
#pragma once
#include "app.h"
#include <string>

class HomePage {
public:
    explicit HomePage(slint::ComponentHandle<AppWindow> ui);
    void set_user(const std::string& name);       // C++ -> UI
    std::string user() const;                      // UI -> C++
private:
    long spawn_path(const char* path);

    slint::ComponentWeakHandle<AppWindow> ui_;
    int clicks_ = 0;
};