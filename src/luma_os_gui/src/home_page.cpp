// // home_page.cpp

#include "home_page.h"
#include "luma_spawn.h"
#include <iostream>
#include "luma_settings_client.h"
#include <cstdlib>

enum { APP_FILES = 0, APP_SETTINGS, APP_TERMINAL, APP_MUSIC };

// HomePage::HomePage(slint::ComponentHandle<AppWindow> ui) : ui_(ui)

HomePage::HomePage(slint::ComponentHandle<AppWindow> ui)
    : ui_(ui), model_(std::make_shared<slint::VectorModel<AppState>>())
{
    apps_ = {
        { "file_manager", "/bin/file_manager", {} },
        { "settings",     "/bin/settings",     {} },
        { "terminal",     "/bin/terminal",     {} },
        { "music",        "/bin/music",        {} },
        { "menu",         "/bin/menu",         {} },
        { "browser",      "/bin/browser",      {} },
        { "photos",       "/bin/photos",       {} },
        { "editor",       "/bin/editor",       {} },
    };
    for (size_t i = 0; i < apps_.size(); i++) model_->push_back(AppState{ 0, false, false });

    auto& s = ui->global<HomeState>();
    s.set_apps(model_);
    s.on_app_clicked([this](int i) { click(i); });
    s.on_app_new_instance([this](int i) { launch(i); });

    s.on_app_new_instance([this](int i) { launch(i); });

    s.on_lock_clicked([this]() { on_lock_clicked(); });
    s.on_power_off_clicked([this]() { on_power_off_clicked(); });
    s.on_internet_clicked([this]() { on_internet_clicked(); });
    s.on_battery_clicked([this]() { on_battery_clicked(); });
    s.on_volume_clicked([this]() { on_volume_clicked(); });

    s.on_wifi_toggled([this](bool on) { std::cout << "[Home] wifi " << on << "\n"; /* kernel call here */ });
    s.on_volume_changed([this](int v) { std::cout << "[Home] volume " << v << "\n"; /* kernel call here */ });
    s.on_mute_toggled([this](bool m) { std::cout << "[Home] mute " << m << "\n"; });
    s.on_lock_now([this] { std::cout << "[Home] lock\n"; });
    s.on_restart_requested([this] { std::cout << "[Home] restart\n"; });
    s.on_shutdown_requested([this] { std::cout << "[Home] shutdown\n"; });

    auto& sc = SettingsClient::get();
    sc.on_changed = [this](const std::string& k, const std::string& v) { apply_setting(k, v); };
    for (auto& [k, v] : sc.cache) apply_setting(k, v);

    // make the tray panel changes flow back to Settings too:
    s.on_volume_changed([](int v)   { SettingsClient::get().set_settings("sound.volume", std::to_string(v)); });
    s.on_mute_toggled  ([](bool m)  { SettingsClient::get().set_settings("sound.muted",  m ? "1" : "0"); });
    s.on_wifi_toggled  ([](bool on) { SettingsClient::get().set_settings("network.wifi", on ? "1" : "0"); });

    ShellClient::get().on_change = [this] { refresh(); };
    reaper_.start(slint::TimerMode::Repeated, std::chrono::milliseconds(500), [this] { reap(); });
    refresh();  
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

int HomePage::window_count(size_t i) const
{
    int n = 0;
    for (auto& [id, w] : ShellClient::get().windows)
        if (w.app_id == apps_[i].app_id) n++;
    return n;
}

void HomePage::refresh()
{
    auto& wins = ShellClient::get().windows;
    for (size_t i = 0; i < apps_.size(); i++) {
        int running = 0;
        bool all_min = true, focused = false;
        for (auto& [id, w] : wins) {
            if (w.app_id != apps_[i].app_id) continue;
            running++;
            if (!w.minimized) all_min = false;
            if (w.focused) focused = true;
        }
        model_->set_row_data(i, AppState{ running, running > 0 && all_min, focused });
    }
}

void HomePage::click(int i)
{
    if (i < 0 || i >= (int)apps_.size())
    {
        std::cout<<"ERROR: Home Page clicked invalid ID = "<<i<<std::endl;
        return;
    }

    auto& sc = ShellClient::get();
    std::cout<<"Home Page clicked, sc.windows Size = "<<sc.windows.size()<<std::endl;

    uint32_t restore = 0, newest = 0;
    for (auto& [id, w] : sc.windows)
    {   
        std::cout<<"Home Page clicked, apps_[i].app_id = "<<apps_[i].app_id<<std::endl;

                           // ascending id
        if (w.app_id != apps_[i].app_id)
        {
            std::cout << "[Home] click continue id = " << id <<"\n"; 
            continue;
        }
        newest = id;
        if (w.minimized && restore == 0)
        {
            std::cout << "[Home] click id = " << id <<"\n";
            restore = id;     // oldest minimized first
        }
    }


    if (restore)
    {
        std::cout << "[Home] restore: " << i<<"\n";
        sc.activate(restore); 
    }      
    else if (newest)
    {
        std::cout << "[Home] newest: " << i<<"\n";
        sc.activate(newest);
    }  
    else if (apps_[i].pids.empty())
    {
        std::cout << "[Home] launch: " << i<<"\n";

        launch(i);              // else: still starting, ignore
    }
    else
    {
        std::cout<<"Home Page clicked else, ID = "<<i<<std::endl;
    }
}

void HomePage::launch(int i)
{
    std::cout << "[Home] launch Request for ID: " <<i<< "\n";

    if (i < 0 || i >= (int)apps_.size()) return;
    long pid = spawn_path(apps_[i].path.c_str());
    if (pid > 0)
    {
        apps_[i].pids.push_back(pid);
        std::cout << "[Home] spawn Request: " << apps_[i].path << " rc=" << pid << "\n";
    }
    else
    {
        std::cout << "[Home] spawn failed: " << apps_[i].path << " rc=" << pid << "\n";
    }
}

void HomePage::reap()
{
    for (auto& a : apps_) {
        for (size_t k = 0; k < a.pids.size(); ) {
            int status = 0;
            pid_t r = waitpid((pid_t)a.pids[k], &status, WNOHANG);
            if (r == (pid_t)a.pids[k] || (r < 0 && errno != EINTR))
                a.pids.erase(a.pids.begin() + k);
            else
                k++;
        }
    }
}

void HomePage::on_lock_clicked()
{
    std::cout<<"HomePage::on_lock_clicked()"<<std::endl;
}

void HomePage::on_power_off_clicked()
{
    std::cout<<"HomePage::on_power_off_clicked()"<<std::endl;
}

void HomePage::on_battery_clicked()
{
    set_battery(87, false);               // TODO: read from the kernel
}

void HomePage::on_internet_clicked()
{
    set_network(true, "Luma-Net");        // TODO: read from the kernel
}

void HomePage::on_volume_clicked()
{
    set_volume_ui(50, false);             // TODO: read from the kernel
}

void HomePage::set_battery(int percent, bool charging)
{
    if (auto ui = ui_.lock()) {
        auto& s = (*ui)->global<HomeState>();
        s.set_battery_percent(percent);
        s.set_battery_charging(charging);
    }
}

void HomePage::set_network(bool on, const std::string& name)
{
    if (auto ui = ui_.lock()) {
        auto& s = (*ui)->global<HomeState>();
        s.set_wifi_on(on);
        s.set_wifi_name(slint::SharedString(name));
    }
}

void HomePage::set_volume_ui(int percent, bool muted)
{
    if (auto ui = ui_.lock()) {
        auto& s = (*ui)->global<HomeState>();
        s.set_volume(percent);
        s.set_muted(muted);
    }
}

void HomePage::apply_setting(const std::string& k, const std::string& v)
{
    std::cout<<"HomePage::apply_setting Key: "<<k<<", value : "<<v<<std::endl;
    auto ui = ui_.lock(); if (!ui) return;
    int n = std::atoi(v.c_str());
    auto& s = (*ui)->global<HomeState>();
    if      (k == "appearance.accent")    (*ui)->global<Theme>().set_accent(n);
    else if (k == "appearance.wallpaper") (*ui)->global<Theme>().set_wallpaper(n);
    else if (k == "sound.volume")         s.set_volume(n);
    else if (k == "sound.muted")          s.set_muted(n != 0);
    else if (k == "network.wifi")         s.set_wifi_on(n != 0);
}

