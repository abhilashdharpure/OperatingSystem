//wifi_proc.c
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "hal/vfs.h"
#include "wifi.h"


// int wifi_get_network(int index, wifi_network_t *out);
// int wifi_get_network_count(void);

wifi_network_t g_wifi_networks[256];
static int g_wifi_network_count= 0;

int wifi_get_network_count(void)
{
    return g_wifi_network_count;
}

int wifi_get_network(int index, wifi_network_t *out)
{
    if (!out)
        return -1;

    if (index < 0 || index >= g_wifi_network_count)
        return -1;

    *out = g_wifi_networks[index];

    return 0;
}

int wifi_is_ready(void)
{
    if(g_wifi_network_count > 0)
    {
        return 1;
    }

    return 0;
}


static int text_read(
    struct file *f,
    void *buf,
    size_t size,
    const char *txt,
    size_t len)
{
    if (f->position >= len)
        return 0;

    size_t n = len - f->position;

    if (n > size)
        n = size;

    memcpy(buf, txt + f->position, n);

    f->position += n;

    return (int)n;
}

static int wifi_read(
    struct file *f,
    void *buf,
    size_t size)
{
    char t[4096];

    int pos = 0;

    int count = wifi_get_network_count();

    if (count < 0)
        count = 0;

    if (count > 32)
        count = 32;

    pos += ksnprintf(
        t + pos,
        sizeof(t) - pos,
        "STATUS=%s\nCOUNT=%d\n",
        wifi_is_ready() ? "READY" : "NOT_READY",
        count
    );


    for (int i = 0; i < count; i++)
    {
        wifi_network_t net;

        memset(&net, 0, sizeof(net));

        if (wifi_get_network(i, &net) != 0)
            continue;

        pos += ksnprintf(
            t + pos,
            sizeof(t) - pos,
            "NETWORK=%s,%d:%d:%d:%d:%d:%d,%d,%d,%d\n",
            net.ssid,
            net.bssid[0],
            net.bssid[1],
            net.bssid[2],
            net.bssid[3],
            net.bssid[4],
            net.bssid[5],
            net.signal,
            net.channel,
            net.security);

        if (pos >= (int)sizeof(t) - 1)
            break;
    }



    ////////////////////////
    // Update Virtual DUmmy data


    virtual_wifi_scan();

    wifi_network_t networks[WIFI_MAX_NETWORKS];

    count = virtual_wifi_get_networks(
        networks,
        WIFI_MAX_NETWORKS
    );

    for (int i = 0; i < count; i++)
    {
        log_info("WIFI",
                "SSID: %s RSSI: %d Channel: %d Security: %d",
                networks[i].ssid,
                networks[i].signal,
                networks[i].channel,
                networks[i].security);

            wifi_network_t net = networks[i];


            pos += ksnprintf(
            t + pos,
            sizeof(t) - pos,
            "NETWORK=%s,%d:%d:%d:%d:%d:%d,%d,%d,%d\n",
            net.ssid,
            net.bssid[0],
            net.bssid[1],
            net.bssid[2],
            net.bssid[3],
            net.bssid[4],
            net.bssid[5],
            net.signal,
            net.channel,
            net.security);
            }


    return text_read(
        f,
        buf,
        size,
        t,
        (size_t)pos
    );
}

static struct file_operations wifi_fops =
{
    .read = wifi_read
};

int wifi_proc_init(void)
{
    VFS_RegisterDevice(
        "/proc/net/wifi",
        &wifi_fops,
        NULL
    );

    log_info("WIFI", "Registered /proc/net/wifi");

    return 0;
}
