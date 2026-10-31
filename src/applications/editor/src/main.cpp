#include "luma_app.h"
#include "app.h"
#include "editor_controller.h"

int main(int argc, char** argv)
{
    std::string path = argc > 1 ? argv[1] : "";
    if (!luma_start("editor", 760, 520)) return 1;

    std::printf("[editor] argc=%d argv1=%s\n", argc, argc > 1 ? argv[1] : "(none)");
    auto ui = EditorWindow::create();
    EditorController editor(ui, path);

    auto& wc = ui->global<WindowControl>();
    wc.on_start_move([]      { if (g_win) g_win->begin_move(); });
    wc.on_toggle_maximize([] { if (g_win) g_win->toggle_maximize(); });
    wc.on_minimize([]        { if (g_win) g_win->minimize(); });
    wc.on_close([]           { if (g_win) g_win->close_window(); });

    ui->run();
    return 0;
}