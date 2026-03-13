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

; Shared GDT for 64-bit long mode, used by both Multiboot2 and Limine entry points.
; Segment selector values (offsets into GDT):
;   Code segment: 0x08
;   Data segment: 0x10

global gdt64
global gdt64_pointer
global gdt64_code_selector
global gdt64_data_selector

; Segment selector constants (offsets into GDT)
; These must match the GDT layout below
%define GDT64_CODE_SEL 0x08
%define GDT64_DATA_SEL 0x10

section .rodata
align 16

gdt64:
    ; Null descriptor (required) - offset 0x00
    dq 0
    ; Code segment - offset 0x08
    ; Execute/Read, 64-bit, present
    ; Bits: Type=Execute/Read (bit 43), S=1 (bit 44), P=1 (bit 47), L=1 (bit 53)
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
    ; Data segment - offset 0x10
    ; Read/Write, present
    ; Bits: Type=Read/Write (bit 41), S=1 (bit 44), P=1 (bit 47)
    dq (1<<41) | (1<<44) | (1<<47)

gdt64_pointer:
    dw gdt64_pointer - gdt64 - 1      ; GDT size (limit)
    dq gdt64                          ; GDT address

; Export selector values as data (for use with `mov ax, [gdt64_code_selector]`)
align 2
gdt64_code_selector:
    dw GDT64_CODE_SEL
gdt64_data_selector:
    dw GDT64_DATA_SEL
