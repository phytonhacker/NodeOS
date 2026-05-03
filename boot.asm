[BITS 32]

; Multiboot header - GRUB megtalálja
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_ALIGN     equ 1<<0
MULTIBOOT_MEMINFO   equ 1<<1
MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .text
global _start

_start:
    cli                 ; interrupts kikapcsolása
    mov esp, stack_top  ; stack beállítása
    
    ; kernel main hívása (C-ből)
    extern kernel_main
    call kernel_main
    
    ; ha visszatér, végtelen ciklus
    hlt
.hang:
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384          ; 16KB stack
stack_top: