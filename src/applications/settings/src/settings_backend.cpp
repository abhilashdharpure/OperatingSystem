#include "settings_backend.h"
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <sys/utsname.h>
#include <unistd.h>
#include "luma_settings_client.h"

using KV = std::map<std::string, std::string>;

static KV read_kv(const char* path)            // "key=value\n" text files exposed by the kernel
{
    KV m;
    int fd = open(path, O_RDONLY);
    if (fd < 0) return m;
    char buf[1024];
    ssize_t n = read(fd, buf, sizeof buf - 1);
    close(fd);
    if (n <= 0) return m;
    buf[n] = 0;
    std::string text(buf);
    for (size_t pos = 0; pos < text.size(); ) {
        size_t e = text.find('\n', pos);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(pos, e - pos);
        pos = e + 1;
        size_t eq = l.find('=');
        if (eq != std::string::npos) m[l.substr(0, eq)] = l.substr(eq + 1);
    }
    return m;
}

static int geti(const KV& m, const char* k, int def = 0)
{
    auto it = m.find(k);
    return it == m.end() ? def : std::atoi(it->second.c_str());
}

// ---- EDID (128-byte base block) ----
struct Edid { bool ok = false; std::string name; int phys_w_mm = 0, phys_h_mm = 0, refresh_mhz = 0; };

static Edid parse_edid(const uint8_t* e, size_t n)
{
    Edid r;
    static const uint8_t hdr[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    if (n < 128 || memcmp(e, hdr, 8) != 0) return r;
    uint8_t sum = 0;
    for (int i = 0; i < 128; i++) sum += e[i];
    if (sum != 0) return r;                                       // checksum must be 0

    uint16_t m = (uint16_t)((e[8] << 8) | e[9]);                  // 3 letters, 5 bits each, 'A' = 1
    char mfg[4] = { char('A' - 1 + ((m >> 10) & 31)), char('A' - 1 + ((m >> 5) & 31)),
                    char('A' - 1 + (m & 31)), 0 };
    r.phys_w_mm = e[21] * 10;                                     // cm -> mm, 0 = unknown
    r.phys_h_mm = e[22] * 10;

    for (int i = 0; i < 4; i++) {                                 // 4 descriptors at offsets 54, 72, 90, 108
        const uint8_t* d = e + 54 + 18 * i;
        if (d[0] || d[1]) {                                       // detailed timing: first one = preferred mode
            if (r.refresh_mhz == 0) {
                long long pclk = (long long)(d[0] | (d[1] << 8)) * 10000LL;           // Hz
                int ha = d[2] | ((d[4] & 0xF0) << 4), hb = d[3] | ((d[4] & 0x0F) << 8);
                int va = d[5] | ((d[7] & 0xF0) << 4), vb = d[6] | ((d[7] & 0x0F) << 8);
                long long total = (long long)(ha + hb) * (va + vb);
                if (total > 0) r.refresh_mhz = (int)(pclk * 1000LL / total);
            }
        } else if (d[3] == 0xFC) {                                // monitor name descriptor
            std::string s((const char*)d + 5, 13);
            r.name = s.substr(0, s.find('\n'));
        }
    }
    r.name = r.name.empty() ? std::string(mfg) : std::string(mfg) + " " + r.name;
    r.ok = true;
    return r;
}

class KernelBackend : public ISettingsBackend {
public:
    bool read_display(DisplayInfo& d) override
    {
        KV m = read_kv("/dev/display0");
        if (!m.empty() && geti(m, "width") > 0) {
             d.width = geti(m, "width");  d.height = geti(m, "height");
            d.bpp = geti(m, "bpp");      d.pitch = geti(m, "pitch");
            d.refresh_mhz = geti(m, "refresh_mhz");
            if (m.count("name")) d.name = m["name"];
        } else if (g.out_w > 0) {
            d.width = g.out_w; d.height = g.out_h; d.bpp = 32;
        } else return false;

        uint8_t raw[128];
        int fd = open("/dev/edid0", O_RDONLY);
        if (fd >= 0) {
            ssize_t n = read(fd, raw, sizeof raw);
            close(fd);
            Edid e = parse_edid(raw, n > 0 ? (size_t)n : 0);
            if (e.ok) {
                d.phys_w_mm = e.phys_w_mm; d.phys_h_mm = e.phys_h_mm;
                if (e.refresh_mhz > 0) d.refresh_mhz = e.refresh_mhz;
                d.name = e.name;
            }
        }
        return true;
    }

    bool read_battery(BatteryInfo& b) override
    {
        KV m = read_kv("/dev/battery0");
#ifdef SETTINGS_FAKE_BATTERY
        if (m.empty()) { b = {true, true, true, 63, 95, 50000, 47000}; return true; }
#endif
        if (m.empty()) { b = BatteryInfo{}; return false; }
        b.present = geti(m, "present") != 0;
        b.percent = geti(m, "percent");
        b.charging = geti(m, "charging") != 0;
        b.ac_online = geti(m, "ac") != 0;
        b.minutes = geti(m, "minutes", -1);
        b.design_mwh = geti(m, "design_mwh");
        b.full_mwh = geti(m, "full_mwh");
        return true;
    }

    std::vector<InfoRow> read_about() override
    {
        std::vector<InfoRow> r;
        r.push_back({"Operating system", "Luma OS"});
        struct utsname u;
        if (uname(&u) == 0) {
            r.push_back({"Kernel", std::string(u.sysname) + " " + u.release});
            r.push_back({"Architecture", u.machine});
        }
        return r;
    }
        
    bool read_network(NetworkInfo& info) override
    {
        info = {};

        int fd = open("/proc/net/info", O_RDONLY);
        if (fd < 0)
            return false;

        char buf[512];
        int n = read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n <= 0)
            return false;

        buf[n] = '\0';

        std::cout << "/proc/net/info:\n"
                << buf << std::endl;

        std::istringstream iss(buf);
        std::string line;

        while (std::getline(iss, line))
        {
            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key   = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "Interface")
            {
                info.interface_name = value;
            }
            else if (key == "Link")
            {
                info.present = (value == "UP");
            }
            else if (key == "IP")
            {
                info.ip_address = value;
                std::cout<<"IP Address is "<<value<<std::endl;

            }
            else if (key == "MAC")
            {
                info.mac_address = value;
                std::cout<<"MAC Address is "<<value<<std::endl;
            }
            else if (key == "Gateway")
            {
                info.gateway = value;
            }
        }

        return true;
    }


    bool ParseWifiNetwork(const std::string& line,
                      WifiNetwork& wifi)
    {
        memset(&wifi, 0, sizeof(wifi));

        int b0, b1, b2, b3, b4, b5;

        int parsed = sscanf(
            line.c_str(),
            "%63[^,],%x:%x:%x:%x:%x:%x,%d,%d,%d",
            wifi.ssid,
            &b0,
            &b1,
            &b2,
            &b3,
            &b4,
            &b5,
            &wifi.rssi,
            &wifi.channel,
            &wifi.security);

        if (parsed != 10)
        {
            std::cout << "Parse failed: parsed="
                    << parsed
                    << " line="
                    << line
                    << std::endl;
            return false;
        }

        snprintf(
            wifi.bssid,
            sizeof(wifi.bssid),
            "%02X:%02X:%02X:%02X:%02X:%02X",
            b0, b1, b2, b3, b4, b5);

        return true;
    }

    bool read_wifi(WifiInfo& info)
    {
        info = {};

        int fd = open("/proc/net/wifi", O_RDONLY);

        if (fd < 0)
        {
            std::cout<<"read_wifi open FD is invalid"<<std::endl;
            return false;
        }

        char buf[4096];

        int n = read(fd, buf, sizeof(buf) - 1);

        close(fd);

        if (n <= 0)
        {
            std::cout<<"read_wifi read FD is invalid"<<std::endl;

            return false;
        }

        buf[n] = '\0';
        info.networks.clear();

        char* line = buf;
        std::cout<<"read_wifi read line : "<<line<<std::endl;

        while (*line)
        {
            char* end = strchr(line, '\n');

            if (end)
                *end = '\0';

            if (strncmp(line, "STATUS=", 7) == 0)
            {
                info.ready =
                    strcmp(line + 7, "READY") == 0;
            }
            else if (strncmp(line, "COUNT=", 6) == 0)
            {
                info.count = atoi(line + 6);

                if (info.count > 32)
                    info.count = 32;
            }
            else if (strncmp(line, "NETWORK=", 8) == 0)
            {
                WifiNetwork network;   
                if (ParseWifiNetwork(
                        std::string(line + 8),
                        network))
                {
                    info.networks.push_back(network);
                }
            }

            if (!end)
                break;

            line = end + 1;
        }

        return true;
    }


    void apply(const std::string& key, const std::string& value) override
    {
        values_[key] = value;
        std::printf("[settings] %s = %s\n", key.c_str(), value.c_str());
        // TODO: forward to the compositor / home (see section 5) or to a kernel device.
        SettingsClient::get().set_settings(key.c_str(), value.c_str());
    }

private:
    KV values_;
};

ISettingsBackend& kernel_backend() { static KernelBackend b; return b; }