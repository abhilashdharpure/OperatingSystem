#include "luma_platform.h"
#include "app.h"
#include "home_page.h"
#include "settings_page.h"
#include <memory>
#include <stdio.h>
#include "luma_settings_client.h"
#include "luma_app.h"

int main()
{

    g_global_hook = [](wl_registry* r, uint32_t n, const char* i, uint32_t) {
        if (!strcmp(i, luma_shell_interface.name))    ShellClient::get().bind(r, n);
        if (!strcmp(i, luma_settings_interface.name)) SettingsClient::get().bind(r, n);
    };
    
    if(!luma_start("luma_home", 0, 0, true))
    {
        return 1;
    }

    auto ui = AppWindow::create();
    HomePage home(ui);               // keep the controller alive until run() returns
    home.set_user("Abhilash");

    SettingsPage settingPage(ui);

    ui->run();
    return 0;
}