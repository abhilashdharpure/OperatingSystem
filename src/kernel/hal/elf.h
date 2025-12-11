#pragma once
#include <stdint.h>
#include <hal/process.h>


typedef struct {
    uintptr_t entry;
    // add program headers, segment info, etc.
} Elf32_Info;


int elf_load_from_vfs(int fd, Elf32_Info *out);
void elf_map_segments(struct Process *p, Elf32_Info *elf);
