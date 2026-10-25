#include "luma_app.h"
#include "luma_settings_client.h"
#include "app.h"
#include "settings_controller.h"
#include <cstring>

int main()
{
    g_global_hook = [](wl_registry* r, uint32_t n, const char* i, uint32_t) {
        if (!strcmp(i, luma_settings_interface.name)) SettingsClient::get().bind(r, n);
    };
    if (!luma_start("settings", 820, 520)) return 1;

    auto ui = SettingsWindow::create();
    SettingsController settings(ui, kernel_backend());
    auto& wc = ui->global<WindowControl>();
    wc.on_start_move([]       { if (g_win) g_win->begin_move(); });
    wc.on_toggle_maximize([]  { if (g_win) g_win->toggle_maximize(); });
    wc.on_minimize([]         { if (g_win) g_win->minimize(); });
    wc.on_close([]            { if (g_win) g_win->close_window(); });
    ui->run();
    return 0;
}