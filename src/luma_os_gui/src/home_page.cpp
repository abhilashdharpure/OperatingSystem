// // home_page.cpp

#include "home_page.h"
#include "process/spawn.h"
#include <iostream>

enum { APP_FILES = 0, APP_SETTINGS, APP_TERMINAL, APP_MUSIC };

HomePage::HomePage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)
{
    auto& s = ui->global<HomeState>();

    apps_[APP_FILES]    = { "/bin/file_manager", 0, [this](bool v){ if (auto u = ui_.lock()) (*u)->global<HomeState>().set_file_manager_open(v); } };
    apps_[APP_SETTINGS] = { "/bin/settings",     0, [this](bool v){ if (auto u = ui_.lock()) (*u)->global<HomeState>().set_settings_open(v); } };
    apps_[APP_TERMINAL] = { "/bin/terminal",     0, [this](bool v){ if (auto u = ui_.lock()) (*u)->global<HomeState>().set_terminal_open(v); } };
    apps_[APP_MUSIC]    = { "/bin/music",        0, [this](bool v){ if (auto u = ui_.lock()) (*u)->global<HomeState>().set_music_open(v); } };

    s.on_open_file_manager([this] { launch(APP_FILES); });
    s.on_open_settings    ([this] { launch(APP_SETTINGS); });
    s.on_open_terminal    ([this] { launch(APP_TERMINAL); });
    s.on_open_music       ([this] { launch(APP_MUSIC); });

    // Check for exited children every 300 ms
    reaper_.start(slint::TimerMode::Repeated, std::chrono::milliseconds(300),
                  [this] { poll_children(); });
}

void HomePage::set_user(const std::string& name)
{
    if (auto ui = ui_.lock())
    {
        (*ui)->global<HomeState>().set_user_name(slint::SharedString(name));
    }
}

std::string HomePage::user() const
{
    if (auto ui = ui_.lock())
    {
        return std::string((*ui)->global<HomeState>().get_user_name());
    }
    return {};
}

void HomePage::launch(int i)
{
    AppSlot& a = apps_[i];
    if (a.pid > 0) {
        std::cout << "[Home] " << a.path << " already running (pid " << a.pid << ")\n";
        return;                                    // one instance per icon
    }
    long pid = spawn_path(a.path.c_str());
    if (pid > 0) {
        a.pid = pid;
        a.set_open(true);
    } else {
        std::cout << "[Home] spawn failed for " << a.path << " rc=" << pid << "\n";
    }
}

void HomePage::poll_children()
{
    for (AppSlot& a : apps_) {
        if (a.pid <= 0) continue;
        int status = 0;
        pid_t r = waitpid((pid_t)a.pid, &status, WNOHANG);   // 0 = still running
        if (r == (pid_t)a.pid || (r < 0 && errno != EINTR)) {
            std::cout << "[Home] " << a.path << " exited (pid " << a.pid
                      << ", status " << status << ")\n";
            a.pid = 0;
            a.set_open(false);                                // removes the highlight
        }
    }
}
// #include "home_page.h"



// HomePage::HomePage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)
// {
//     std::cout<<"Home page: construct!"<<std::endl;

//     auto& state = ui->global<HomeState>();
//     state.set_greeting("Hello from C++");

//     // UI -> C++ callbacks. Capture 'this' and use the weak handle inside.
//     state.on_say_hello([this](slint::SharedString name)
//     {
//         if (auto ui = ui_.lock()) {
//             ++clicks_;
//             auto& s = (*ui)->global<HomeState>();
//             s.set_clicks(clicks_);
//             s.set_greeting(slint::SharedString("Hello, " + std::string(name)));
//         }
//     });

//     state.on_open_settings([this]
//     {
//         std::cout<<"Home page: on_open_settings called!"<<std::endl;

//         if (auto ui = ui_.lock()) {
//             (*ui)->set_current_page(1);
//             auto &s = (*ui)->global<HomeState>();
//             s.set_settings_open(true);
//         }
//     });
    

//     state.on_open_file_manager([this]
//     {
//         std::cout<<"Home page: on_open_file_manager called!"<<std::endl;

//         long pid = spawn_path("/bin/file_manager");

//         if (pid > 0) {
//             if (auto ui = ui_.lock()) {
//                 auto &s = (*ui)->global<HomeState>();
//                 s.set_file_manager_open(true);
//             }
//         }
//     });


//     state.on_open_terminal([this]
//     {
//         std::cout<<"Home page: terminal click called!"<<std::endl;

//         long pid = spawn_path("/bin/client");

//         if (pid > 0) {
//             if (auto ui = ui_.lock()) {
//                 auto &s = (*ui)->global<HomeState>();
//                 s.set_terminal_open(true);
//             }
//         }
//     });


//     state.on_open_music([this]
//     {
//         std::cout<<"Home page: open_music click called!"<<std::endl;

//         long pid = spawn_path("/bin/client");

//         if (pid > 0) {
//             if (auto ui = ui_.lock()) {
//                 auto &s = (*ui)->global<HomeState>();
//                 s.set_music_open(true);
//             }
//         }
//     });
// }

// void HomePage::set_user(const std::string& name)
// {
//     if (auto ui = ui_.lock())
//     {
//         (*ui)->global<HomeState>().set_user_name(slint::SharedString(name));
//     }
// }

// std::string HomePage::user() const
// {
//     if (auto ui = ui_.lock())
//     {
//         return std::string((*ui)->global<HomeState>().get_user_name());
//     }
//     return {};
// }

// long HomePage::spawn_path(const char* path)
// {
//     char* argv[] = { (char*)path, NULL };
//     char* envp[] = { (char*)"XDG_RUNTIME_DIR=/tmp",
//                      (char*)"WAYLAND_DISPLAY=wayland-0", NULL };
//     return syscall(SYS_spawn, path, argv, envp, 0);   /* 0: init reaps via wait4 */
// }
