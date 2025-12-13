# TinyOS Boot Module

Boot loader interface and early initialization code.

## Contents

- `multiboot.asm` - Multiboot2 header (to be created in T2)
- `boot.asm` - Boot entry assembly code (to be created in T3)

## Public Interface

Headers in `include/boot/`:
- `multiboot.h` - Multiboot2 structures and definitions

## Responsibilities

- Implement Multiboot2 protocol header
- Set up initial stack
- Clear BSS section
- Pass control to C kernel entry point
- Preserve bootloader information (multiboot magic, info pointer)
