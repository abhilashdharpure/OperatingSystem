// home_page.cpp
#include "home_page.h"


#define SYS_spawn   0x1001
#define SPAWN_DETACH 1

HomePage::HomePage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)
{
    std::cout<<"Home page: construct!"<<std::endl;

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
        std::cout<<"Home page: on_open_settings called!"<<std::endl;

        if (auto ui = ui_.lock()) (*ui)->set_current_page(1);
    });
    

    state.on_open_file_manager([this] {
        std::cout<<"Home page: on_open_file_manager called!"<<std::endl;

        long comp_pid = spawn_path("/bin/file_manager");
    });


    state.on_open_terminal([this] {
        std::cout<<"Home page: terminal click called!"<<std::endl;

        long comp_pid = spawn_path("/bin/client");
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

long HomePage::spawn_path(const char* path)
{
    char* argv[] = { (char*)path, NULL };
    char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp",
                     (char*)"WAYLAND_DISPLAY=wayland-0", NULL };
    return syscall(SYS_spawn, path, argv, envp, 0);   /* 0: init reaps via wait4 */
}
