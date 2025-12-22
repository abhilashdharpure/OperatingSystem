; src/kernel/arch/x86_64/io_asm.asm
BITS 64

global x64_inb
x64_inb:
    mov dx, di
    xor eax, eax
    in al, dx
    ret

global x64_outb
x64_outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

global x64_inw
x64_inw:
    mov dx, di
    xor eax, eax
    in ax, dx
    ret

global x64_outw
x64_outw:
    mov dx, di
    mov ax, si
    out dx, ax
    ret


global x64_enable_interrupts
x64_enable_interrupts:
    sti
    ret

global x64_disable_interrupts
x64_disable_interrupts:
    cli
    ret
