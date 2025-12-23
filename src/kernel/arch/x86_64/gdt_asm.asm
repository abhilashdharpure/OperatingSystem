; gdt_asm.asm

[bits 32]

; void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
global i686_GDT_Load
i686_GDT_Load:
    
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame
    
    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf

.reload_cs:

    ; reload data segments
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax,
    mov ss, ax

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret

; boot-time GDT for long mode
global gdt64_desc

section .data
align 8
gdt64:
    dq 0x0000000000000000    ; null
    dq 0x00209A0000000000    ; 0x08: 64-bit code
    dq 0x0000920000000000    ; 0x10: data
gdt64_end:

gdt64_desc:
    dw gdt64_end - gdt64 - 1
    dq gdt64


