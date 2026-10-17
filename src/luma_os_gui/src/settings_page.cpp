// settings_page.cpp
#include "settings_page.h"

SettingsPage::SettingsPage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)
{
    std::cout<<"Setting page: construct!"<<std::endl;

    auto& state = ui->global<SettingsState>();
    state.set_greeting("Hello from Settings Page");

    // UI -> C++ callbacks. Capture 'this' and use the weak handle inside.
    state.on_say_hello([this](slint::SharedString name) {
        if (auto ui = ui_.lock()) {
            ++clicks_;
            auto& s = (*ui)->global<SettingsState>();
            s.set_clicks(clicks_);
            s.set_greeting(slint::SharedString("Setting:, " + std::string(name)));
        }
    });

    state.on_open_home([this]
    {
        std::cout<<"Setting page: on_open_home called!"<<std::endl;
        if (auto ui = ui_.lock()) (*ui)->set_current_page(0);
    });
}

void SettingsPage::set_user(const std::string& name)
{
    if (auto ui = ui_.lock()) (*ui)->global<SettingsState>().set_user_name(slint::SharedString(name));
}

std::string SettingsPage::user() const
{
    if (auto ui = ui_.lock())
        return std::string((*ui)->global<SettingsState>().get_user_name());
    return {};
}