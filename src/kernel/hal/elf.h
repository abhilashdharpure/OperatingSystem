#pragma once
#include <stdint.h>
#include <hal/process.h>

/* e_ident indexes */
#define EI_MAG0   0
#define EI_MAG1   1
#define EI_MAG2   2
#define EI_MAG3   3
#define EI_CLASS  4

/* Magic numbers */
#define ELFMAG0   0x7F
#define ELFMAG1   'E'
#define ELFMAG2   'L'
#define ELFMAG3   'F'

/* Class */
#define ELFCLASS32 1
#define ELFCLASS64 2

/* Machine */
#define EM_X86_64 62

/* Program header types */
#define PT_NULL   0
#define PT_LOAD   1

#define USER_MMAP_BASE  0x50000000
#define USER_HEAP_START 0x60000000ULL
#define USER_HEAP_END   0x70000000ULL   // 256MB heap space


/* Minimal 64-bit ELF header */
typedef struct {
    unsigned char e_ident[16];
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

/* Minimal 64-bit program header */
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
