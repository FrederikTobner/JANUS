# TinyOS Boot Module

Boot loader interface and early initialization code.

## Responsibilities

- Implement Multiboot2 protocol header
- Set up initial stack
- Clear BSS section
- Pass control to C kernel entry point
- Preserve bootloader information (multiboot magic, info pointer)

## Contents

- `multiboot.asm` - Multiboot2 header
- `boot.asm` - Boot entry assembly code

## Public Interface

Headers in `include/boot/`:
- `multiboot.h` - Multiboot2 structures and definitions
