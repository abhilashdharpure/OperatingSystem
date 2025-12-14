// #include "vmm.h"

// #define KERNEL_BASE 0xC0000000  // change if different

// uint32_t virt_to_phys(void *virt)
// {
//     return (uint32_t)virt - KERNEL_BASE;
// }

// void *phys_to_virt(uint32_t phys)
// {
//     return (void *)(phys + KERNEL_BASE);
// }

// void map_page_user(void *page_dir, uintptr_t virt, uintptr_t phys,
//                    int user, int writable)
// {
//     // TEMPORARY: Identity mapping
//     // When paging is implemented, replace this!
// }
