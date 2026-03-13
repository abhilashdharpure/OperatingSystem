#pragma once
#include <stdint.h>
#include <hal/process.h>


/* ELF constants used in your code */
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS64 2
#define EM_X86_64 62

/* Class */
#define ELFCLASS32 1
// #define ELFCLASS64 2

/* Machine */

/* Program header types */
#define PT_NULL   0
#define PT_LOAD   1

#define USER_MMAP_BASE  0x50000000
#define USER_HEAP_START 0x60000000ULL
#define USER_HEAP_END   0x70000000ULL   // 256MB heap space


/* e_ident indexes */
enum {
    EI_MAG0 = 0, EI_MAG1, EI_MAG2, EI_MAG3,
    EI_CLASS, EI_DATA, EI_VERSION, EI_OSABI,
    EI_ABIVERSION, EI_PAD
};


/* Minimal ELF64 definitions (match System V AMD64 ABI) */

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint64_t      e_entry;
    uint64_t      e_phoff;
    uint64_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;