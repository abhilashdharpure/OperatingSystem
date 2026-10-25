#pragma once
#include "app.h"
#include "luma_shell_client.h"
#include <memory>
#include <string>
#include <vector>

class HomePage {
public:
    explicit HomePage(slint::ComponentHandle<AppWindow> ui);
    void set_user(const std::string& name);       // C++ -> UI
    std::string user() const;                      // UI -> C++

private:
    struct AppDef {
        std::string app_id, path;
        std::vector<long> pids;                    // launched, not yet reaped
    };

    int  window_count(size_t i) const;
    void refresh();
    void click(int i);
    void launch(int i);
    void reap();

    void on_lock_clicked();
    void on_power_off_clicked();
    void on_internet_clicked();
    void on_battery_clicked();
    void on_volume_clicked();

    void set_battery(int percent, bool charging);
    void set_network(bool on, const std::string& name);
    void set_volume_ui(int percent, bool muted);

    void apply_setting(const std::string&, const std::string&);

    slint::ComponentWeakHandle<AppWindow>          ui_;
    std::vector<AppDef>                            apps_;
    std::shared_ptr<slint::VectorModel<AppState>>  model_;
    slint::Timer                                   reaper_;
};