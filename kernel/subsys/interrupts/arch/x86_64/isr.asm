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

; x86_64 interrupt/exception entry stubs.
;
; One stub per vector (0-255). Each stub normalises the stack frame so that the
; C dispatcher always sees an identical layout: vectors that do not push a
; hardware error code get a synthetic zero, and every stub pushes its vector
; number. The common tail saves all general-purpose registers and calls
; interrupts_dispatch(frame). Fatal handlers never return, but a full restore +
; iretq path is provided for completeness / future recoverable vectors.

bits 64

extern interrupts_dispatch
global isr_stub_table

section .text

; Per-vector entry for exceptions WITHOUT a CPU-pushed error code.
%macro ISR_NOERR 1
isr_stub_%1:
    push qword 0            ; synthetic error code
    push qword %1           ; vector number
    jmp isr_common
%endmacro

; Per-vector entry for exceptions WHERE the CPU already pushed an error code.
%macro ISR_ERR 1
isr_stub_%1:
    push qword %1           ; vector number (error code already on stack)
    jmp isr_common
%endmacro

; Generate all 256 stubs. Vectors 8, 10, 11, 12, 13, 14, 17, 21, 29, 30 push an
; error code; all others do not.
; 29 = #VC (VMM Communication, AMD SEV-ES), 30 = #SX (Security Exception).
%assign v 0
%rep 256
    %if v == 8 || v == 10 || v == 11 || v == 12 || v == 13 || v == 14 || v == 17 || v == 21 || v == 29 || v == 30
        ISR_ERR v
    %else
        ISR_NOERR v
    %endif
    %assign v v+1
%endrep

; Common tail: save GPRs (r15 pushed last -> lowest address -> frame.r15),
; pass the frame pointer to C, then (unreachable for fatal faults) restore.
isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    cld
    mov rdi, rsp            ; arg0 = interrupt_frame_t *
    call interrupts_dispatch

    ; Retained for completeness; interrupts_dispatch is __noreturn today.
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16            ; discard vector + error_code
    iretq

; Table of stub entry points, consumed by idt.c to fill the 256 gates.
section .rodata
align 8
isr_stub_table:
%assign v 0
%rep 256
    dq isr_stub_ %+ v
    %assign v v+1
%endrep
