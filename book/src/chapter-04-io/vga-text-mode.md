# VGA Text Mode

> **TODO: Implement VGA text mode display**

## What We'll Build

VGA text mode provides an 80x25 grid of colored characters. It's the classic way to display text on PC screens before graphics modes existed. Simple, fast, and perfect for our kernel.

## Why VGA?

1. **Simple memory-mapped interface**: Write to 0xB8000
2. **No initialization needed**: BIOS sets it up for us
3. **Fast**: Direct memory writes, no complex protocols
4. **Colors**: 16 foreground and 8 background colors
5. **Universal**: Works on all x86 PCs

## Topics to Cover

- VGA text buffer memory layout (0xB8000)
- Character attributes (color, blinking)
- Cursor management (hardware cursor via I/O ports)
- Scrolling implementation
- `kprintf()` redirection to screen

## Implementation Plan

1. Map VGA buffer at 0xB8000
2. Implement `vga_putchar()` with color support
3. Add cursor positioning and movement
4. Implement scrolling when buffer fills
5. Create screen clearing and formatting functions
6. Integrate with `kprintf()`

---

**Next: [Keyboard Input](keyboard-input.md)**
