# Serial Output

> **TODO: Implement serial output via COM1 port**

## What We'll Build

The serial port (COM1) is the simplest output device on x86. It's a character-at-a-time interface used for debugging and logging. Modern computers don't have physical serial ports, but QEMU emulates one perfectly.

## Why Serial First?

1. **Simple**: Just write bytes to I/O port 0x3F8
2. **Reliable**: No complex initialization like VGA
3. **Essential for debugging**: Works even when screen output fails
4. **QEMU support**: `-serial stdio` redirects to terminal

## Topics to Cover

- I/O port access (`outb`, `inb` instructions)
- COM1 port initialization (baud rate, line control)
- Buffered vs. unbuffered output
- Formatting functions (`printf`-like)
- Integration with kernel logging

## Implementation Plan

1. Write low-level port I/O functions
2. Initialize COM1 (115200 baud, 8N1)
3. Implement `serial_putchar()` and `serial_write()`
4. Create `kprintf()` for formatted output
5. Test with QEMU's serial output

---

**Next: [VGA Text Mode](vga-text-mode.md)**
