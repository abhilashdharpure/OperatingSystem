// #include "elf.h"
// #include "fat.h"
// #include "memory.h"
// #include "paging.h"
// #include "kmalloc.h"
// #include "string.h"


// UserProcess* ELF_ReadUser(Partition* part, const char* path)
// {
//     uint8_t* headerBuffer = kmalloc(4096);
//     uint8_t* loadBuffer   = kmalloc(4096);

//     FAT_File* fd = FAT_Open(part, path);
//     if (!fd) return NULL;

//     // Read ELF header
//     if (FAT_Read(part, fd, sizeof(ELFHeader), headerBuffer) != sizeof(ELFHeader)) {
//         return NULL;
//     }

//     ELFHeader* header = (ELFHeader*)headerBuffer;

//     // Allocate new address space
//     uint32_t* pd = paging_create_userspace_pd();

//     // Load program headers
//     FAT_Read(part, fd, header->ProgramHeaderTablePosition - sizeof(ELFHeader), headerBuffer);
//     FAT_Read(part, fd,
//              header->ProgramHeaderTableEntryCount * header->ProgramHeaderTableEntrySize,
//              headerBuffer);

//     // For each program header
//     for (uint32_t i = 0; i < header->ProgramHeaderTableEntryCount; i++) {
//         ELFProgramHeader* ph = (ELFProgramHeader*)
//             (headerBuffer + i * header->ProgramHeaderTableEntrySize);

//         if (ph->Type != ELF_PROGRAM_TYPE_LOAD)
//             continue;

//         uint32_t dst = ph->VirtualAddress;
//         uint32_t memsz = ph->MemorySize;
//         uint32_t filesz = ph->FileSize;

//         // Map memory region as user-space
//         for (uint32_t off = 0; off < memsz; off += 4096) {
//             paging_map_user_page(pd, dst + off);
//         }

//         // Zero the region
//         memset((void*)dst, 0, memsz);

//         // Seek file to segment offset
//         FAT_Close(fd);
//         fd = FAT_Open(part, path);

//         uint32_t remaining = ph->Offset;
//         while (remaining > 0) {
//             uint32_t chunk = min(remaining, 4096u);
//             FAT_Read(part, fd, chunk, loadBuffer);
//             remaining -= chunk;
//         }

//         // Read segment content
//         remaining = filesz;
//         uint8_t* writePtr = (uint8_t*)dst;

//         while (remaining > 0) {
//             uint32_t chunk = min(remaining, 4096u);
//             FAT_Read(part, fd, chunk, loadBuffer);
//             memcpy(writePtr, loadBuffer, chunk);
//             writePtr += chunk;
//             remaining -= chunk;
//         }
//     }

//     FAT_Close(fd);

//     // Create user stack
//     const uint32_t USER_STACK_TOP = 0xBFFFF000;
//     for (int i = 0; i < 8; i++) {
//         paging_map_user_page(pd, USER_STACK_TOP - i * 4096);
//     }

//     // Create process object
//     UserProcess* proc = kmalloc(sizeof(UserProcess));
//     proc->entry = header->ProgramEntryPosition;
//     proc->user_stack = USER_STACK_TOP;
//     proc->page_directory = pd;

//     return proc;
// }
