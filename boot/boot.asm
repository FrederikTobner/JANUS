; Boot Entry Point
; TinyOS Boot Assembly Code
;
; Copyright (C) 2025 by Frederik Tobner
; This file is part of TinyOS.
; Licensed under the GNU Affero General Public License v3.0
; See https://www.gnu.org/licenses/agpl-3.0.en.html

; This code is executed immediately after the bootloader transfers control.
; It sets up the execution environment and calls the C kernel entry point.

global _start
extern kernel_main

; Reserve stack space in BSS section
section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

section .text
bits 32                     ; Bootloader puts us in 32-bit protected mode

_start:
    ; At this point:
    ; - EAX contains the multiboot2 magic value (0x36d76289)
    ; - EBX contains the physical address of the multiboot information structure
    ; - CPU is in 32-bit protected mode
    ; - Interrupts are disabled
    ; - Paging is disabled
    
    ; Disable interrupts (in case bootloader didn't)
    cli
    
    ; Set up stack pointer
    ; Stack grows downward, so we point ESP to the top
    mov esp, stack_top
    
    ; Reset EFLAGS register
    ; Clear all flags to ensure clean state
    push 0
    popf
    
    ; Clear direction flag (for string operations)
    ; Ensures string operations increment (not decrement) pointers
    cld
    
    ; Save multiboot information for kernel
    ; We need to preserve these before calling C code
    ; C calling convention (cdecl): arguments pushed right-to-left
    push ebx                ; Multiboot info structure pointer (2nd arg)
    push eax                ; Multiboot magic number (1st arg)
    
    ; Call the C kernel entry point
    ; void kernel_main(uint32_t magic, struct multiboot_info *mbi)
    call kernel_main
    
    ; If kernel_main returns (which it shouldn't), halt the CPU
    ; This prevents executing random memory
.hang:
    cli                     ; Disable interrupts
    hlt                     ; Halt CPU until next interrupt
    jmp .hang               ; If we somehow wake up, halt again
