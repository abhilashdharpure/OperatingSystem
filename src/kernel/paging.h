#pragma once
#include <stdint.h>
#include <stddef.h>
#include <hal/process.h>


// 64-bit page directory = PML4
typedef struct {
    uint64_t pd_phys;   // physical address of PML4
    uint64_t *pd_virt;  // kernel virtual address of PML4
} page_dir_t;

// Flags
#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_RW        (1ULL << 1)
#define PAGE_USER      (1ULL << 2)

// Core mapping functions (64-bit)
int      map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags);
// void     unmap_page(uint64_t *pml4, uint64_t va);
uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va);
// int      map_region(uint64_t *pml4, uint64_t va, uint64_t pa_start, uint64_t len, uint64_t flags);

// User page table creation
page_dir_t create_user_pd(void);
void       clone_kernel_mappings(uint64_t *user_pml4);

// Enter user mode
void enter_user_mode_from_process(struct Process *p);


// #ifndef PAGING_H
// #define PAGING_H

// #include <stdint.h>
// #include <stddef.h>

// /* Page flags (32-bit x86) */
// #define PAGE_SIZE    4096
// #define PAGE_PRESENT  0x001
// #define PAGE_RW       0x002
// #define PAGE_USER     0x004

// /* GDT selectors: adjust to match your GDT */
// #define KERNEL_CS 0x08
// #define KERNEL_DS 0x10
// #define USER_CS   0x1B   /* (selector index << 3) | 3  typically 0x18|3 => 0x1B */
// #define USER_DS   0x23   /* (selector index << 3) | 3  typically 0x20|3 => 0x23 */


// /* page size */
// #define KERNEL_VMA  0xC0000000

// extern uint32_t *kernel_page_directory;       // virtual address
// extern uint32_t  kernel_page_directory_phys;  // physical address

// void paging_bootstrap(void);
// // void write_cr3(uint32_t phys);
// void enable_paging(void);

// typedef struct {
//     uint32_t *pd_virt;
//     uint32_t  pd_phys;
// } page_dir_t;

// /* Basic API you can call from kernel code */
// void paging_init(void); /* optional: call once at boot */
// int map_page(uint32_t *pd_phys_ptr, uintptr_t va, uintptr_t pa, uint32_t flags);
// void map_identity_page(uint32_t* pd, uintptr_t pa);
// void unmap_page(uint32_t *page_directory, uint32_t va);
// int map_region(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa_start, uint32_t len, uint32_t flags);
// // uint32_t *create_user_pd(void); /* create PD for user process (physical-page-backed), returns pointer to PD phys (kernel-virt access) */
// page_dir_t create_user_pd(void);

// int paging_map_user(uint32_t va, uint32_t len); /* convenience used in your start_userspace() */
// void clone_kernel_mappings(uint32_t *new_pd);
// // void write_cr3(uint32_t pa);

// #endif
