// settings_page.h
#pragma once
#include "app.h"
#include <string>

class SettingsPage {
public:
    explicit SettingsPage(slint::ComponentHandle<AppWindow> ui);
    void set_user(const std::string& name);       // C++ -> UI
    std::string user() const;                      // UI -> C++
private:
    slint::ComponentWeakHandle<AppWindow> ui_;
    int clicks_ = 0;
};