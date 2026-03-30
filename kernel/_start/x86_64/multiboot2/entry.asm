; Copyright (C) 2025 by Frederik Tobner                                     
;                                                                           
; This file is part of JANUS.                                              
;                                                                           
; Permission to use, copy, modify, and distribute this software and its     
; documentation under the terms of the GNU Affero General Public License is 
; hereby granted.                                                           
; No representations are made about the suitability of this software for    
; any purpose.                                                              
; It is provided "as is" without express or implied warranty.               
; See the <https://www.gnu.org/licenses/agpl-3.0.en.html>                   
; GNU Affero General Public License                                         
; License for more details.     

; Multiboot2 entry point for x86_64.
;
; Executed immediately after GRUB transfers control.  Transitions from
; 32-bit protected mode to 64-bit long mode, then calls the C kernel.
;
; Page table setup lives in paging.asm for readability.

global _start_multiboot2
extern multiboot2_stash_bootinfo
extern kernel_main
extern gdt64_pointer
extern setup_page_tables            ; paging.asm
extern p4_table                     ; paging.asm

; GDT segment selectors
%define GDT64_CODE_SEL  0x08
%define GDT64_DATA_SEL  0x10

; Control register bits
%define CR4_PAE         (1 << 5)
%define CR0_PAGING      (1 << 31)

; EFER MSR
%define MSR_EFER        0xC0000080
%define EFER_LONG_MODE  (1 << 8)

section .bss
alignb 16
stack_bottom:
    resb 16384                      ; 16 KiB stack
stack_top:

section .text.multiboot2_entry
; 32-bit Protected Mode
bits 32

_start_multiboot2:
    ; At entry:
    ; - EAX = multiboot2 magic (0x36d76289)
    ; - EBX = multiboot info pointer
    ; - 32-bit protected mode, paging disabled

    cli

    ; Save multiboot info
    mov edi, eax                    ; magic
    mov esi, ebx                    ; info pointer

    ; Set up stack
    mov esp, stack_top

    ; Set up page tables and enable long mode
    call setup_page_tables          ; paging.asm — identity-maps 4 GB
    call enable_paging

    ; Load 64-bit GDT
    lgdt [gdt64_pointer]

    ; Jump to 64-bit code
    jmp GDT64_CODE_SEL:long_mode_start

; Enable long mode by configuring PAE, EFER and paging.
; Requires page tables already loaded into p4_table.
enable_paging:
    ; Load P4 into CR3
    mov eax, p4_table
    mov cr3, eax

    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, CR4_PAE
    mov cr4, eax

    ; Enable Long Mode in EFER MSR
    mov ecx, MSR_EFER
    rdmsr
    or eax, EFER_LONG_MODE
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, CR0_PAGING
    mov cr0, eax

    ret

; 64-bit Long Mode
bits 64

long_mode_start:
    ; Set up segment registers
    mov ax, GDT64_DATA_SEL
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; RSP already set, RDI/RSI have magic/info from 32-bit setup
    ; Stash bootinfo for later use by boot_init in kernel_main
    call multiboot2_stash_bootinfo

    ; Enter kernel
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
