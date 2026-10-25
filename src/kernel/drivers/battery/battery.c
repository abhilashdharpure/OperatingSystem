#include "battery.h"
#include "stdint.h"
#include "stdio.h"
#include "debug.h"
#include "hal/file.h"
#include <stdio.h>
#include "stddef.h"
#include <arch/x86_64/irq.h>

struct battery_state g_battery;

#ifdef SETTINGS_FAKE_BATTERY
    if (geti(m, "present") == 0) { b = {true, true, true, 63, 95, 50000, 47000}; return true; }
#endif

static int text_read(struct file *f, void *buf, size_t size, const char *txt, size_t len)
{
    if (f->position >= len) return 0;
    size_t n = len - f->position;
    if (n > size) n = size;
    memcpy(buf, txt + f->position, n);
    f->position += n;
    return (int)n;
}

static int battery_read(struct file *f, void *buf, size_t size)
{
    char t[160];
    ksnprintf(t, sizeof t, "present=%d\npercent=%d\ncharging=%d\nac=%d\nminutes=%d\ndesign_mwh=%d\nfull_mwh=%d\n",
              g_battery.present, g_battery.percent, g_battery.charging, g_battery.ac,
              g_battery.minutes, g_battery.design_mwh, g_battery.full_mwh);
    return text_read(f, buf, size, t, strlen(t));
}
static struct file_operations battery_fops = { .read = battery_read };


void battery_devices_init(void)
{
    VFS_RegisterDevice("/dev/battery0", &battery_fops, NULL);
}