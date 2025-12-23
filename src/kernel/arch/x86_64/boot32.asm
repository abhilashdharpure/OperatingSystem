[BITS 32]
global multiboot_entry
extern pml4_table
extern gdt64_desc
extern kernel_entry

multiboot_entry:
    cli

    ; ; Debug marker
    ; mov dx, 0x3F8
    ; mov al, '1'
    ; out dx, al

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Load PML4
    mov eax, pml4_table
    mov cr3, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_desc]

    ; Assume GRUB passed pointer in EBX
    mov edi, ebx        ; first arg in 32-bit lower half of RDI

    ; proper far jump to 64-bit
    jmp 0x08:kernel_entry
