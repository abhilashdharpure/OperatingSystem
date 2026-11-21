#pragma once

#include <stdint.h>

typedef struct { uint8_t bus, dev, fn; uint16_t vendor, device; uint8_t cls, sub, prog; } pci_dev_t;
