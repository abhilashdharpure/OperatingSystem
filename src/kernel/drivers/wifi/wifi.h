#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

#define WIFI_MAX_NETWORKS 16
#define WIFI_MAX_SSID_LEN 32

#define WIFI_SECURITY_OPEN 0
#define WIFI_SECURITY_WEP  1
#define WIFI_SECURITY_WPA  2
#define WIFI_SECURITY_WPA2 3
#define WIFI_SECURITY_WPA3 4

typedef struct
{
    char ssid[WIFI_MAX_SSID_LEN + 1];

    uint8_t bssid[6];

    int8_t signal;

    uint8_t channel;

    uint8_t security;

} wifi_network_t;

typedef struct
{
    int (*init)(void);

    int (*scan)(void);

    int (*get_networks)(
        wifi_network_t *networks,
        int max_networks
    );

} wifi_driver_t;

int wifi_init(void);

int wifi_scan(void);

int wifi_get_networks(
    wifi_network_t *networks,
    int max_networks
);

#endif
