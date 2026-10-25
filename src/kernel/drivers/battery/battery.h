#pragma once
#include "stdint.h"
#include <string.h>

struct battery_state { int present, percent, charging, ac, minutes, design_mwh, full_mwh; };
extern struct battery_state g_battery;       // updated by the ACPI driver


void battery_devices_init(void);
