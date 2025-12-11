#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"
#include "process.h"
#include "pmm.h"
#include "debug.h"
#include "paging.h"

#define ELF_MAGIC0 0x7F
#define ELF_MAGIC1 'E'
#define ELF_MAGIC2 'L'
#define ELF_MAGIC3 'F'
#define PAGE_SIZE 4096
// #define USER_STACK_TOP 0x80000000
#define USER_STACK_TOP 0x080ff000
#define USER_STACK_SIZE 0x20000

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

#define PT_LOAD 1

/* helpers you must have or implement */
extern uint32_t virt_to_phys(void *v);   /* returns physical address of kernel-virtual pointer */
extern void write_cr3(uint32_t pd_phys); /* wrapper assembly for mov cr3, ... */

/* GDT selectors: adjust to match your GDT */
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x1B   /* (selector index << 3) | 3  typically 0x18|3 => 0x1B */
#define USER_DS   0x23   /* (selector index << 3) | 3  typically 0x20|3 => 0x23 */

void enter_user_mode_from_process(Process *p)
{
    if (!p || !p->page_directory) {
        log_error("EXEC", "enter_user_mode: invalid process or missing page_directory");
        return;
    }

    uint32_t pd_phys = virt_to_phys(p->page_directory);
    if (!pd_phys) {
        log_error("EXEC", "enter_user_mode: virt_to_phys failed");
        return;
    }

    log_info("EXEC", "enter_user_mode_from_process write_cr3");

    /* Load the process page directory (CR3) */
    write_cr3(pd_phys);
    log_info("EXEC", "enter_user_mode_from_process After write_cr3");


    /* Load user data selectors into DS/ES/FS/GS while still in ring0.
       Loading SS must be done by IRET, so we push SS/ESP in the iret frame below. */
    __asm__ volatile (
        "movw %[udsel], %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        : /* no outputs */
        : [udsel] "i" (USER_DS)
        : "ax", "memory"
    );

    log_info("EXEC", "enter_user_mode_from_process switch to user mode............");


    /* Now switch to user mode: build an iret frame and iret.
       Interrupts should be disabled while manipulating the stack frame */
    __asm__ volatile (
        "cli\n\t"                         /* disable interrupts during the switch */
        "pushl %[udsel]\n\t"             /* SS (user data selector) */
        "pushl %[esp]\n\t"               /* ESP (user stack top) */
        "pushf\n\t"                      /* EFLAGS */
        "pushl %[ucsel]\n\t"             /* CS (user code selector) */
        "pushl %[eip]\n\t"               /* EIP (entry point) */
        "iret\n\t"
        :
        : [udsel] "r" ((uint32_t)USER_DS),
          [esp]   "r" ((uint32_t)p->regs.esp),
          [ucsel] "r" ((uint32_t)USER_CS),
          [eip]   "r" ((uint32_t)p->regs.eip)
        : "memory"
    );

    /* we should never reach here in the kernel if iret succeeded */
    log_critical("EXEC", "enter_user_mode: iret returned unexpectedly");
    for(;;);
}

// uint32_t get_mapped_phys(uint32_t *pd, uint32_t va)
// {
//     uint32_t pd_index = va >> 22;
//     uint32_t pt_index = (va >> 12) & 0x3FF;

//     uint32_t pd_entry = pd[pd_index];
//     if (!(pd_entry & PAGE_PRESENT)) return 0;

//     uint32_t *pt = (uint32_t*)phys_to_virt(pd_entry & 0xFFFFF000);
//     uint32_t pt_entry = pt[pt_index];
//     if (!(pt_entry & PAGE_PRESENT)) return 0;

//     return pt_entry & 0xFFFFF000;
// }



pid_t exec_elf_mem(void *data, size_t size)
{
    Elf32_Ehdr *eh = (Elf32_Ehdr *)data;

    if (eh->e_entry == 0) {
        log_error("EXEC", "ELF entry is zero");
        return -1;
    }

    Process *p = process_create("user");
    if (!p) return -1;

    uint32_t *pd = create_user_pd();
    if (!pd) return -1;

    clone_kernel_mappings(pd);
    p->page_directory = pd;

    Elf32_Phdr *ph = (Elf32_Phdr *)((uint8_t*)data + eh->e_phoff);

    for (int i = 0; i < eh->e_phnum; i++, ph++) {
        if (ph->p_type != PT_LOAD) continue;

        uint32_t start = ph->p_vaddr & ~(PAGE_SIZE - 1);
        uint32_t end   = (ph->p_vaddr + ph->p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        for (uint32_t va = start; va < end; va += PAGE_SIZE) {
            uint32_t pa = pmm_alloc_page();
            if (!pa) return -1;

            memset(phys_to_virt(pa), 0, PAGE_SIZE);

            if (map_page(pd, va, pa, PAGE_PRESENT | PAGE_USER | PAGE_RW) != 0)
                return -1;
        }

        memcpy((void *)ph->p_vaddr,
               (uint8_t *)data + ph->p_offset,
               ph->p_filesz);
    }

    // #############################################################
    // p->regs.eip = eh->e_entry;
    // p->regs.esp = process_setup_stack(p);
    // #############################################################


    // #######################  New code ######################################

    uint32_t stack_pa = pmm_alloc_page();
    map_page(pd, USER_STACK_TOP - USER_STACK_SIZE, stack_pa,
            PAGE_PRESENT | PAGE_USER | PAGE_RW);

    p->regs.esp = USER_STACK_TOP;
    log_info("EXEC", "ELF loaded p->regs.esp =0x%x", p->regs.esp );
    log_info("EXEC", "ELF loaded entry=0x%x", eh->e_entry);

    p->regs.eip = eh->e_entry;

    // #############################################################


    enter_user_mode_from_process(p);

    return 0;
}
