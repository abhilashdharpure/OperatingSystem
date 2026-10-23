#include "luma_platform.h"
#include "app.h"
#include "home_page.h"
#include "settings_page.h"
#include <memory>
#include <stdio.h>


int main()
{
    auto platform = std::make_unique<LumaPlatform>();
    if (!platform->connect()) {
        printf("[luma_os_gui] cannot connect to wayland\n");
        return 1;
    }
    slint::platform::set_platform(std::move(platform));
    g_app_id = "luma_home";
    g_fill_output = true;                   // size = screen, no resize edges

    auto ui = AppWindow::create();
    HomePage home(ui);               // keep the controller alive until run() returns
    home.set_user("Abhilash");

    SettingsPage settingPage(ui);

    ui->run();
    return 0;
}