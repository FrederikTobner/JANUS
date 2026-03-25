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

; Early paging setup for the Multiboot2 boot path.
;
; Identity-maps the first 4 GB of physical memory using 2 MB huge pages.
; This is enough to cover the kernel (loaded near 1 MB), the legacy VGA
; text buffer (0xB8000), and any framebuffer GRUB may provide (typically
; in the 0xFD000000 range on QEMU).
;
; Called from entry.asm in 32-bit protected mode before the switch to
; long mode.

global setup_page_tables
global p4_table

; ─────────────────────────────────────────────────────────
; Page table flags
; ─────────────────────────────────────────────────────────

%define PT_PRESENT      (1 << 0)
%define PT_WRITABLE     (1 << 1)
%define PT_HUGE         (1 << 7)        ; PS bit — 2 MB page

%define PT_PRESENT_RW   (PT_PRESENT | PT_WRITABLE)
%define PT_HUGE_RW      (PT_PRESENT | PT_WRITABLE | PT_HUGE)

; Page geometry
%define P2_ENTRY_COUNT  2048            ; 4 tables × 512 entries
%define P2_TABLE_COUNT  4               ; one per GB

; ─────────────────────────────────────────────────────────
; BSS — page table storage (must be page-aligned)
; ─────────────────────────────────────────────────────────

section .bss

alignb 4096
p4_table:   resb 4096
p3_table:   resb 4096
p2_tables:  resb 4096 * P2_TABLE_COUNT  ; one P2 table per GB (0–4 GB)

; ─────────────────────────────────────────────────────────
; 32-bit — Page Table Setup
; ─────────────────────────────────────────────────────────

section .text.multiboot2_paging
bits 32

; Identity-map first 4 GB using 2 MB huge pages.
;
; Clobbers: EAX, ECX, EDI (all saved/restored around multiboot info).
; Preserves: EDI and ESI across the call (multiboot magic & info pointer).
setup_page_tables:
    ; Save multiboot info (edi = magic, esi = info pointer)
    push edi
    push esi

    ; P4[0] → P3
    mov eax, p3_table
    or eax, PT_PRESENT_RW
    mov [p4_table], eax

    ; P3[0..3] → four P2 tables, one per GB
    mov edi, p3_table
    mov eax, p2_tables
    or eax, PT_PRESENT_RW
    mov ecx, P2_TABLE_COUNT
.wire_p3:
    mov [edi], eax
    add eax, 4096               ; next P2 table
    add edi, 8                  ; next P3 entry
    dec ecx
    jnz .wire_p3

    ; Fill P2 entries with 2 MB huge pages covering 0x0 – 0xFFFFFFFF
    mov edi, p2_tables
    xor ecx, ecx
.fill_p2:
    mov eax, ecx
    shl eax, 21                 ; physical address = index × 2 MB
    or eax, PT_HUGE_RW
    mov [edi], eax              ; lower 32 bits
    mov dword [edi + 4], 0      ; upper 32 bits (address < 4 GB)
    add edi, 8
    inc ecx
    cmp ecx, P2_ENTRY_COUNT
    jb .fill_p2

    ; Restore multiboot info
    pop esi
    pop edi
    ret
