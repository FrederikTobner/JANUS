; Multiboot2 Header
; This file defines the Multiboot2 header that allows GRUB to load our kernel.
; It must be located in the first 32KB of the kernel image.

section .multiboot
align 8

multiboot_start:
    ; Multiboot2 header magic number
    dd 0xe85250d6
    
    ; Architecture: 0 = i386/x86_64 protected mode
    dd 0
    
    ; Header length (from start to end tag)
    dd multiboot_end - multiboot_start
    
    ; Checksum: -(header magic + architecture + header_length)
    dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))

; Information request tag
; Requests specific information from the bootloader
align 8
information_request_tag_start:
    dw 1                             ; type = 1 (information request)
    dw 0                             ; flags = 0 (required)
    dd information_request_tag_end - information_request_tag_start ; size
    
    ; Request types:
    dd 4                             ; Basic memory information
    dd 6                             ; Memory map
    dd 8                             ; Framebuffer info
    dd 9                             ; ELF sections
information_request_tag_end:

; End tag (required)
; Marks the end of the Multiboot2 header
align 8
    dw 0                             ; type = 0 (end tag)
    dw 0                             ; flags = 0
    dd 8                             ; size = 8 bytes
multiboot_end:
