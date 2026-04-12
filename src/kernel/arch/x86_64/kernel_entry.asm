[BITS 64]
SECTION .boot64_stub

global kernel_low_entry
global kernel_entry

extern _boot_stack_top
extern kernel_main_entry

%macro putc_imm 1
    mov al, %1
    mov dx, 0x3F8
    out dx, al
%endmacro

kernel_low_entry:
    putc_imm 'L'
    jmp kernel_entry

kernel_entry:
    cli
    putc_imm 'E'

    mov     rsp, _boot_stack_top
    and     rsp, -16
    xor     rbp, rbp

    putc_imm 'S'

    ; RDI has multiboot info
    call    kernel_main_entry

hang:
    putc_imm 'H'
    hlt
    jmp     hang
