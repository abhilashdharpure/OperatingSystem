#pragma once
#include <string>
#include <vector>
#include "luma_platform.h"                 // g.out_w / g.out_h from wl_output (fallback)

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

struct NetworkInfo {
    bool present = false;
    std::string ip_address;
    std::string mac_address;
    std::string interface_name ;
    std::string gateway;
    std::vector<std::string> wifi_networks;
};

struct WifiNetwork
{
    char ssid[64];
    char bssid[18];

    int rssi;
    int channel;
    int security;
};

struct WifiInfo
{
    bool ready;
    int count;

    std::vector<WifiNetwork> networks;
};



class ISettingsBackend {
public:
    virtual ~ISettingsBackend() = default;
    virtual bool read_display(DisplayInfo&) = 0;
    virtual bool read_battery(BatteryInfo&) = 0;
    virtual std::vector<InfoRow> read_about() = 0;
    virtual bool read_network(NetworkInfo&) = 0;
    virtual bool read_wifi(WifiInfo& info) = 0;

    virtual void apply(const std::string& key, const std::string& value) = 0;
};

ISettingsBackend& kernel_backend();