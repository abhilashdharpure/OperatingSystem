#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

/* Page flags (32-bit x86) */
#define PAGE_PRESENT  0x001
#define PAGE_RW       0x002
#define PAGE_USER     0x004

/* page size */
#define PAGE_SIZE 4096U

/* Basic API you can call from kernel code */
void paging_init(void); /* optional: call once at boot */
uint32_t *create_page_directory(void); /* returns physical address? see comment */
int map_page(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa, uint32_t flags);
int map_region(uint32_t *pd_phys_ptr, uint32_t va, uint32_t pa_start, uint32_t len, uint32_t flags);
uint32_t *create_user_pd(void); /* create PD for user process (physical-page-backed), returns pointer to PD phys (kernel-virt access) */
void switch_page_dir(uint32_t *pd_phys_ptr);
int paging_map_user(uint32_t va, uint32_t len); /* convenience used in your start_userspace() */
void clone_kernel_mappings(uint32_t *new_pd);
// void* phys_to_virt(uint32_t pa);

void write_cr3(uint32_t pa);
/* debug helpers */
void dump_pde_pte(uint32_t va);

#endif
