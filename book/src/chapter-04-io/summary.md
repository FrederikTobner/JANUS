# Summary

At this point, our kernel can:

- Output text via the serial port (COM1)
- Display text on screen (VGA text mode)

Keyboard input is next—we've sketched the plan, but we haven't implemented it yet.

### What's Next

In the next chapter, we'll tackle memory management, the foundation for everything that follows. We'll need to:

- Parse the Multiboot memory map
- Implement physical memory allocation
- Set up virtual memory with page tables
- Enable memory protection

> **Challenges**
>
> 1. **Serial Port Configuration** (Implementation): Modify the serial port initialization code to support different baud rates and serial ports
> 2. **VGA Graphics Mode** (Research & Implementation): Research how to switch the VGA card into graphics mode (e.g., 320x200 with 256 colors). Implement a basic graphics mode driver that can set pixels and draw simple shapes.
> 3. **Keyboard Layouts** (Research): Research how different keyboard layouts (e.g., QWERTY, AZERTY, DVORAK) affect scancode translation. Write a brief summary of how you would implement support for multiple layouts in the keyboard driver.

---

**Next: [Chapter 5: Memory Management](../chapter-05-memory/README.md)**
