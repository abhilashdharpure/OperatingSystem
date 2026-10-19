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

    slint::ComponentWeakHandle<AppWindow>          ui_;
    std::vector<AppDef>                            apps_;
    std::shared_ptr<slint::VectorModel<AppState>>  model_;
    slint::Timer                                   reaper_;
};