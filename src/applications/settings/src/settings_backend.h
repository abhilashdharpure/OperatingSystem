#pragma once
#include <string>
#include <vector>

struct InfoRow { std::string key, value; };

struct DisplayInfo {
    int width = 0, height = 0, bpp = 0, pitch = 0;
    int refresh_mhz = 0;                 // 0 = unknown
    int phys_w_mm = 0, phys_h_mm = 0;    // 0 = unknown
    std::string name;
};

struct BatteryInfo {
    bool present = false, charging = false, ac_online = false;
    int percent = 0;
    int minutes = -1;                    // -1 = unknown
    int design_mwh = 0, full_mwh = 0;
};

class ISettingsBackend {
public:
    virtual ~ISettingsBackend() = default;
    virtual bool read_display(DisplayInfo&) = 0;
    virtual bool read_battery(BatteryInfo&) = 0;
    virtual std::vector<InfoRow> read_about() = 0;
    virtual void apply(const std::string& key, const std::string& value) = 0;
};

ISettingsBackend& kernel_backend();