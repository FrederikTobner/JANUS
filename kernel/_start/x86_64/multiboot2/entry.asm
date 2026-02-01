; Multiboot2 Entry Point
; JANUS Boot Assembly Code
;
; Copyright (C) 2025 by Frederik Tobner
; This file is part of JANUS.
; Licensed under the GNU Affero General Public License v3.0
; See https://www.gnu.org/licenses/agpl-3.0.en.html

; This code is executed immediately after GRUB/Limine transfers control via Multiboot2.
; It sets up 64-bit long mode and calls the C kernel entry point.

global _start_multiboot2
extern kernel_main
extern gdt64_pointer

; GDT segment selectors
%define GDT64_CODE_SEL 0x08
%define GDT64_DATA_SEL 0x10

section .bss
alignb 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

; Page tables for long mode (must be page-aligned)
alignb 4096
p4_table:   resb 4096
p3_table:   resb 4096
p2_table:   resb 4096

section .text.multiboot2_entry
bits 32

_start_multiboot2:
    ; At entry:
    ; - EAX = multiboot2 magic (0x36d76289)
    ; - EBX = multiboot info pointer
    ; - 32-bit protected mode, paging disabled

    cli
    
    ; Save multiboot info
    mov edi, eax            ; magic
    mov esi, ebx            ; info pointer

    ; Set up stack
    mov esp, stack_top

    ; Set up page tables and enable long mode
    call setup_page_tables
    call enable_paging

    ; Load 64-bit GDT
    lgdt [gdt64_pointer]

    ; Jump to 64-bit code
    jmp GDT64_CODE_SEL:long_mode_start

; Identity-map first 2MB using 2MB huge pages
setup_page_tables:
    ; P4[0] -> P3
    mov eax, p3_table
    or eax, 0b11            ; Present + Writable
    mov [p4_table], eax

    ; P3[0] -> P2
    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    ; P2[0] -> 0x0 (2MB huge page)
    mov eax, 0x0
    or eax, 0b10000011      ; Present + Writable + Huge
    mov [p2_table], eax

    ; P2[1] -> 0x200000 (second 2MB, for kernel at 1MB)
    mov eax, 0x200000
    or eax, 0b10000011
    mov [p2_table + 8], eax

    ret

enable_paging:
    ; Load P4 into CR3
    mov eax, p4_table
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enable Long Mode in EFER
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

bits 64
long_mode_start:
    ; Set up segment registers
    mov ax, GDT64_DATA_SEL
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; RSP already set, RDI/RSI have magic/info
    ; RDX = NULL (no framebuffer info for Multiboot2)
    xor rdx, rdx
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
