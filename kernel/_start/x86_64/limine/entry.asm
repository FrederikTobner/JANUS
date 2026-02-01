; Limine Entry Point
; JANUS Boot Assembly Code
;
; Copyright (C) 2025 by Frederik Tobner
; This file is part of JANUS.
; Licensed under the GNU Affero General Public License v3.0
; See https://www.gnu.org/licenses/agpl-3.0.en.html

; This code is executed by Limine after it transfers control.
; Limine already sets up:
; - 64-bit long mode
; - Higher-half paging (maps kernel at virtual address from ELF)
; - A valid stack (64KB or per stack size request)
; - A valid GDT with segments at 0x28 (code) and 0x30 (data)
;
; We simply call kernel_main with appropriate arguments.

global _start_limine
extern kernel_main

; Limine bootloader magic (passed to kernel_main for protocol detection)
%define LIMINE_MAGIC 0x4C494D494E450000  ; "LIMINE\0\0" as u64

section .text.limine_entry
bits 64

_start_limine:
    ; Limine enters here in 64-bit mode with:
    ; - Paging enabled (higher-half mapped per ELF program headers)
    ; - Interrupts disabled
    ; - A valid stack provided by Limine (64KB)
    ; - A valid GDT with code at 0x28 and data at 0x30
    ;
    ; We're already running at the higher-half virtual address.
    ; Use Limine's stack and GDT for now - we can set up our own later.

    ; Set up arguments for kernel_main(u64 loader_magic, void* info, void* fb_info)
    ; RDI = magic value (Limine magic)
    ; RSI = pointer to HHDM response (for boot_info_init to get HHDM offset)
    ; RDX = pointer to framebuffer response (for TTY framebuffer)
    mov rdi, LIMINE_MAGIC
    
    ; Get HHDM response pointer from the request structure
    lea rax, [rel limine_hhdm_request]
    mov rsi, [rax + 40]        ; Offset 40 = response pointer (after id[4] + revision)

    ; Get framebuffer response pointer from the request structure
    lea rax, [rel limine_framebuffer_request]
    mov rdx, [rax + 40]        ; Offset 40 = response pointer

    ; Call kernel_main
    call kernel_main

    ; If kernel_main returns, halt forever
.hang:
    cli
    hlt
    jmp .hang

; Import limine request symbols
extern limine_base_revision
extern limine_hhdm_request
extern limine_framebuffer_request
