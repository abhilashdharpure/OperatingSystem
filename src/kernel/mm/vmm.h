#pragma once
#include <stdint.h>

uint32_t virt_to_phys(void *virt);
void *phys_to_virt(uint32_t phys);
void map_page_user(void *page_dir, uintptr_t virt, uintptr_t phys,
                   int user, int writable);