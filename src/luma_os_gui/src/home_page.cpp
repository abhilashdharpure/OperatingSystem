// home_page.cpp
#include "home_page.h"

HomePage::HomePage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)
{
    auto& state = ui->global<HomeState>();
    state.set_greeting("Hello from C++");

    // UI -> C++ callbacks. Capture 'this' and use the weak handle inside.
    state.on_say_hello([this](slint::SharedString name) {
        if (auto ui = ui_.lock()) {
            ++clicks_;
            auto& s = (*ui)->global<HomeState>();
            s.set_clicks(clicks_);
            s.set_greeting(slint::SharedString("Hello, " + std::string(name)));
        }
    });

    state.on_open_settings([this] {
        if (auto ui = ui_.lock()) (*ui)->set_current_page(1);
    });
}

void HomePage::set_user(const std::string& name)
{
    if (auto ui = ui_.lock()) (*ui)->global<HomeState>().set_user_name(slint::SharedString(name));
}

std::string HomePage::user() const
{
    if (auto ui = ui_.lock())
        return std::string((*ui)->global<HomeState>().get_user_name());
    return {};
}