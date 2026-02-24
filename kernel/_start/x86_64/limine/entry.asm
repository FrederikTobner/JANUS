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

; This code is executed by Limine after it transfers control.
; Limine already sets up:
; - 64-bit long mode
; - Higher-half paging (maps kernel at virtual address from ELF)
; - A valid stack (64KB or per stack size request)
; - A valid GDT with segments at 0x28 (code) and 0x30 (data)
;
; We simply call kernel_main with appropriate arguments.
;
; Note: Limine request structures are defined in common/limine_requests.c
; for portability across architectures.

global _start_limine
extern kernel_main

; Limine bootloader magic (passed to kernel_main for protocol detection)
%define LIMINE_MAGIC 0x4C494D494E450000  ; "LIMINE\0\0" as u64

; Import limine request symbols from limine_requests.c
extern limine_hhdm_request
extern limine_framebuffer_request

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
    ; C struct layout: id[4] (32 bytes) + revision (8 bytes) + response (8 bytes)
    ; Offset to response = 32 + 8 = 40 bytes
    lea rax, [rel limine_hhdm_request]
    mov rsi, [rax + 40]        ; response pointer

    ; Get framebuffer response pointer from the request structure
    lea rax, [rel limine_framebuffer_request]
    mov rdx, [rax + 40]        ; response pointer

    ; Call kernel_main
    call kernel_main

    ; If kernel_main returns, halt forever
.hang:
    cli
    hlt
    jmp .hang
