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
#define PAGE_SIZE            4096ULL

#define USER_START      0x0000000040000000ULL
#define USER_END        0x0000000044000000ULL
#define USER_STACK_TOP  USER_END
#define USER_STACK_SIZE 0x00200000ULL
#define USER_MMAP_BASE  0x50000000ULL 

#define PAGE_SIZE 0x1000
#define TICKS_PER_SEC 1000             // e.g. 1ms tick

// Prot flags (mirror Linux for future compatibility)
#define PROT_READ   0x1
#define PROT_WRITE  0x2

// Map flags
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

#define MSR_FS_BASE 0xC0000100

// linux values
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003

// For now, kernel low memory is identity-mapped: VA == PA for all paging
// structures and low RAM. This matches your current boot paging setup.
static inline void *phys_to_virt(uint64_t pa)
{
    return (void *)(uintptr_t)pa;
}

static inline uint64_t virt_to_phys(void *va)
{
    return (uint64_t)(uintptr_t)va;
}

void debug_dump_va_mapping(uint64_t *pml4, uint64_t va);

// Core mapping functions (64-bit)
int      map_page(uint64_t *pml4, uint64_t va, uint64_t pa, uint64_t flags);
void     unmap_page(uint64_t *pml4, uint64_t va);
uint64_t get_mapped_phys(uint64_t *pml4, uint64_t va);
int set_page_flags(uint64_t *pml4, uint64_t va, uint64_t flags);

// int      map_region(uint64_t *pml4, uint64_t va, uint64_t pa_start, uint64_t len, uint64_t flags);

// User page table creation
page_dir_t create_user_pd(void);
void       clone_kernel_mappings(uint64_t *user_pml4);
void       clone_kernel_mappings_for_user(uint64_t *user_pml4);

// Enter user mode
void enter_user_mode_from_process(struct Process *p);