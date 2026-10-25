#include "settings_controller.h"
#include <chrono>
#include <cmath>
#include <cstdio>

static slint::SharedString S(const std::string& s) { return slint::SharedString(s); }

std::shared_ptr<slint::Model<InfoLine>> SettingsController::to_model(const std::vector<InfoRow>& rows)
{
    std::vector<InfoLine> v;
    for (auto& r : rows) v.push_back(InfoLine{ S(r.key), S(r.value) });
    return std::make_shared<slint::VectorModel<InfoLine>>(std::move(v));
}

SettingsController::SettingsController(slint::ComponentHandle<SettingsWindow> ui, ISettingsBackend& be)
    : ui_(ui), be_(be)
{
    auto& s = ui->global<SettingsState>();
    s.on_setting_changed([this](slint::SharedString k, slint::SharedString v) {
        be_.apply(std::string(k), std::string(v));
    });
    s.on_page_opened([this](int p) {
        switch (p) {
            case 0: refresh_display(); break;
            case 3: refresh_network(); break;
            case 4: refresh_battery(); break;
            case 6: refresh_about();   break;
        }
    });

    refresh_display(); refresh_network(); refresh_battery(); refresh_about();
    timer_.start(slint::TimerMode::Repeated, std::chrono::seconds(5), [this] { refresh_battery(); });
}

void SettingsController::refresh_display()
{
    DisplayInfo d;
    std::vector<InfoRow> rows;
    char b[96];
    if (be_.read_display(d)) {
        std::snprintf(b, sizeof b, "%d x %d", d.width, d.height);          rows.push_back({"Resolution", b});
        std::snprintf(b, sizeof b, "%d-bit", d.bpp);                       rows.push_back({"Colour depth", b});
        if (d.refresh_mhz > 0) std::snprintf(b, sizeof b, "%.0f Hz", d.refresh_mhz / 1000.0);
        else                   std::snprintf(b, sizeof b, "Unknown");
        rows.push_back({"Refresh rate", b});
        if (d.phys_w_mm > 0 && d.phys_h_mm > 0) {
            double diag = std::sqrt((double)d.phys_w_mm * d.phys_w_mm + (double)d.phys_h_mm * d.phys_h_mm) / 25.4;
            std::snprintf(b, sizeof b, "%d x %d mm (%.1f in)", d.phys_w_mm, d.phys_h_mm, diag);
        } else std::snprintf(b, sizeof b, "Unknown");
        rows.push_back({"Physical size", b});
        rows.push_back({"Display", d.name.empty() ? "Unknown" : d.name});
        rows.push_back({"Interface scale", "100%"});
    } else {
        rows.push_back({"Display", "No display information"});
    }
    with_state([&](auto& s) { s.set_display_info(to_model(rows)); });
}

void SettingsController::refresh_battery()
{
    BatteryInfo b;
    be_.read_battery(b);

    std::string t;
    if (b.present && b.minutes >= 0) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "About %d h %d min %s", b.minutes / 60, b.minutes % 60,
                      b.charging ? "until full" : "remaining");
        t = buf;
    } else if (b.present) {
        t = "Estimating time...";
    }

    std::vector<InfoRow> rows;
    if (b.present && b.design_mwh > 0 && b.full_mwh > 0) {
        char buf[64];
        std::snprintf(buf, sizeof buf, "%d%% of design capacity", b.full_mwh * 100 / b.design_mwh);
        rows.push_back({"Battery health", buf});
    }

    with_state([&](auto& s) {
        s.set_battery_present(b.present);
        s.set_battery_percent(b.percent);
        s.set_battery_charging(b.charging);
        s.set_ac_online(b.ac_online);
        s.set_battery_time(S(t));
        s.set_power_info(to_model(rows));
    });
}

void SettingsController::refresh_network()
{
    with_state([&](auto& s) {
        s.set_network_status("No network adapter");
        s.set_network_info(to_model({}));
    });
}

void SettingsController::refresh_about()
{
    with_state([&](auto& s) { s.set_about_info(to_model(be_.read_about())); });
}