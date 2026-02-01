; Limine Boot Protocol Requests
; JANUS Boot Assembly Code
;
; Copyright (C) 2025 by Frederik Tobner
; This file is part of JANUS.
; Licensed under the GNU Affero General Public License v3.0
; See https://www.gnu.org/licenses/agpl-3.0.en.html

; Limine request structures for Limine v8+ (barebones protocol).
; These are placed in a dedicated section that Limine scans at boot time.
; See: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md

global limine_requests_start_marker
global limine_requests_end_marker
global limine_base_revision

; External entry point
extern _start_limine

; Limine common magic (first 2 qwords of every request ID)
%define LIMINE_COMMON_MAGIC_0 0xc7b1dd30df4c8b88
%define LIMINE_COMMON_MAGIC_1 0x0a82e883a194f07b

;------------------------------------------------------------------------------
; Limine Requests Section
; Must be marked with start/end markers for Limine to find
;------------------------------------------------------------------------------
section .limine_requests_start
align 8
limine_requests_start_marker:
    dq 0xf6b8f4b39de7d1ae    ; LIMINE_REQUESTS_START_MARKER[0]
    dq 0xfab91a6940fcb9cf    ; LIMINE_REQUESTS_START_MARKER[1]
    dq 0x785c6ed015d3e316    ; LIMINE_REQUESTS_START_MARKER[2]
    dq 0x181e920a7852b9d9    ; LIMINE_REQUESTS_START_MARKER[3]

section .limine_requests

; Base revision - tells Limine which protocol revision we support
; Format: magic[0], magic[1], revision
; For Limine v8+, revision 3 is the current protocol
align 8
limine_base_revision:
    dq 0xf9562b2d5c95a6c8    ; LIMINE_BASE_REVISION magic[0]
    dq 0x6a7b384944536bdc    ; LIMINE_BASE_REVISION magic[1]
    dq 3                      ; Revision 3 (Limine v8+)

; Entry point request - tells Limine where to jump
; ID: LIMINE_COMMON_MAGIC, 0x13d86c035a1cd3e1, 0x2b0caa89d8f3026a
align 8
limine_entry_point_request:
    dq LIMINE_COMMON_MAGIC_0  ; ID[0] - common magic
    dq LIMINE_COMMON_MAGIC_1  ; ID[1] - common magic
    dq 0x13d86c035a1cd3e1     ; ID[2] - entry point specific
    dq 0x2b0caa89d8f3026a     ; ID[3] - entry point specific
    dq 0                      ; Revision
    dq 0                      ; Response (filled by Limine)
    dq _start_limine          ; Entry point

; Stack size request - request a specific stack size
; ID: LIMINE_COMMON_MAGIC, 0x224ef0460a8e8926, 0xe1cb0fc25f46ea3d
align 8
limine_stack_size_request:
    dq LIMINE_COMMON_MAGIC_0  ; ID[0] - common magic
    dq LIMINE_COMMON_MAGIC_1  ; ID[1] - common magic
    dq 0x224ef0460a8e8926     ; ID[2] - stack size specific
    dq 0xe1cb0fc25f46ea3d     ; ID[3] - stack size specific
    dq 0                      ; Revision
    dq 0                      ; Response
    dq 65536                  ; Stack size: 64KB

; HHDM (Higher Half Direct Map) request - get the HHDM offset
; ID: LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b
; This is required to access physical memory (like VGA buffer at 0xB8000)
global limine_hhdm_request
align 8
limine_hhdm_request:
    dq LIMINE_COMMON_MAGIC_0  ; ID[0] - common magic
    dq LIMINE_COMMON_MAGIC_1  ; ID[1] - common magic
    dq 0x48dcf1cb8ad2b852     ; ID[2] - HHDM specific
    dq 0x63984e959a98244b     ; ID[3] - HHDM specific
    dq 0                      ; Revision
limine_hhdm_response:
    dq 0                      ; Response pointer (filled by Limine)

; Framebuffer request - get a framebuffer for graphical output
; ID: LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b
global limine_framebuffer_request
align 8
limine_framebuffer_request:
    dq LIMINE_COMMON_MAGIC_0  ; ID[0] - common magic
    dq LIMINE_COMMON_MAGIC_1  ; ID[1] - common magic
    dq 0x9d5827dcd881dd75     ; ID[2] - framebuffer specific
    dq 0xa3148604f6fab11b     ; ID[3] - framebuffer specific
    dq 0                      ; Revision
limine_framebuffer_response:
    dq 0                      ; Response pointer (filled by Limine)

section .limine_requests_end
align 8
limine_requests_end_marker:
    dq 0xadc0e0531bb10d03    ; LIMINE_REQUESTS_END_MARKER[0]
    dq 0x9572709f31764c62    ; LIMINE_REQUESTS_END_MARKER[1]
