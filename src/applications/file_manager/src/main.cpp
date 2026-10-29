#include "luma_platform.h"
#include "app.h"
#include "file_manager.h"
#include <memory>
#include <stdio.h>

int main()
{
    auto platform = std::make_unique<LumaPlatform>();
    if (!platform->connect()) {
        printf("[files] cannot connect to wayland\n");
        return 1;
    }
    slint::platform::set_platform(std::move(platform));

    auto ui = AppWindow::create();

    FileManager fm(ui, demo_backend());     // swap for your kernel-backed backend later
    fm.navigate("/");


    auto& wc = ui->global<WindowControl>();
    wc.on_start_move([]      { if (g_win) g_win->begin_move(); });
    wc.on_toggle_maximize([] { if (g_win) g_win->toggle_maximize(); });
    wc.on_minimize([]        { if (g_win) g_win->minimize(); });
    wc.on_close([]           { if (g_win) g_win->close_window(); });

    g_app_id = "file_manager";
    ui->run();
    return 0;
}