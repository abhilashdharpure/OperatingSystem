#pragma once
#include "app.h"
#include "settings_backend.h"
#include <memory>

class SettingsController {
public:
    SettingsController(slint::ComponentHandle<SettingsWindow> ui, ISettingsBackend& be);

private:
    void refresh_display();
    void refresh_battery();
    void refresh_network();
    void refresh_about();
    static std::shared_ptr<slint::Model<InfoLine>> to_model(const std::vector<InfoRow>& rows);

    template <class F> void with_state(F&& f) {
        if (auto ui = ui_.lock()) f((*ui)->global<SettingsState>());
    }

    slint::ComponentWeakHandle<SettingsWindow> ui_;
    ISettingsBackend& be_;
    slint::Timer      timer_;
};