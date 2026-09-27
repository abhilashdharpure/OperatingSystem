#pragma once
#include <stdint.h>
#include <hal/process.h>

/* Basic ELF types (System V AMD64 ABI) */
typedef uint64_t Elf64_Addr;   /* Unsigned program address */
typedef uint64_t Elf64_Off;    /* Unsigned file offset */
typedef uint16_t Elf64_Half;   /* Unsigned medium integer */
typedef uint32_t Elf64_Word;   /* Unsigned integer */
typedef int32_t  Elf64_Sword;  /* Signed integer */
typedef uint64_t Elf64_Xword;  /* Unsigned long integer */
typedef int64_t  Elf64_Sxword; /* Signed long integer */

/* Object file types (e_type) */
#define ET_NONE   0   /* No file type */
#define ET_REL    1   /* Relocatable file */
#define ET_EXEC   2   /* Executable file */
#define ET_DYN    3   /* Shared object (PIE) */
#define ET_CORE   4   /* Core file */


/* ELF magic and class */
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS32 1
#define ELFCLASS64 2
#define EM_X86_64 62

/* Program header types */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2   /* needed for relocations */

/* Dynamic section tags */
#define DT_NULL     0
#define DT_RELA     7
#define DT_RELASZ   8

/* Relocation types */
#define R_X86_64_NONE      0
#define R_X86_64_RELATIVE  8

/* e_ident indexes */
enum {
    EI_MAG0 = 0, EI_MAG1, EI_MAG2, EI_MAG3,
    EI_CLASS, EI_DATA, EI_VERSION, EI_OSABI,
    EI_ABIVERSION, EI_PAD
};

#define EI_NIDENT 16

/* ELF header */
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

/* Program header */
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

/* Dynamic section entry */
typedef struct {
    int64_t d_tag;
    union {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
} Elf64_Dyn;

/* Relocation entry with addend */
typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t  r_addend;
} Elf64_Rela;

/* Relocation macros */
#define ELF64_R_SYM(i)   ((i) >> 32)
#define ELF64_R_TYPE(i)  ((uint32_t)(i))

/* Relocation entry without addend (REL) */
typedef struct {
    Elf64_Addr r_offset;  /* Location to apply the relocation action */
    Elf64_Xword r_info;   /* Symbol table index and type of relocation */
} Elf64_Rel;

/* Dynamic section tags for REL relocations */
#define DT_REL      17   /* Address of Rel relocation table */
#define DT_RELSZ    18   /* Size in bytes of Rel relocation table */
#define DT_RELENT   19   /* Size of each Rel entry */
