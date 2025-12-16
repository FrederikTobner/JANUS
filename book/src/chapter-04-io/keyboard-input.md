# Keyboard Input

> **TODO: Implement PS/2 keyboard input handling**

## What We'll Build

Reading keyboard input requires handling interrupts from the PS/2 keyboard controller. We'll set up interrupt handlers, decode scancodes, and provide a simple input interface.

## Why Keyboard Input Matters

1. **User interaction**: Can't have an OS without input
2. **Interrupt handling practice**: First real interrupt we'll handle
3. **State management**: Shift, Ctrl, Alt modifiers
4. **Scancode translation**: Hardware codes → ASCII characters

## Topics to Cover

- PS/2 keyboard controller (port 0x60, 0x64)
- Interrupt setup (IDT entry for IRQ1)
- Scancode sets (Set 1 vs Set 2)
- Key press vs. key release detection
- Modifier key handling (Shift, Ctrl, Alt)
- Circular buffer for key events

## Implementation Plan

1. Set up IDT entry for keyboard interrupt (IRQ1)
2. Write keyboard interrupt handler
3. Implement scancode → ASCII translation table
4. Handle modifier keys (Shift, Ctrl, Alt)
5. Create circular buffer for keypress queue
6. Implement `getchar()` and `gets()` functions
7. Test with simple keyboard echo program

## Challenges

- **Scancode complexity**: Different layouts, special keys
- **Interrupt safety**: Can fire at any time
- **Race conditions**: Buffer access from interrupt and main code
- **International keyboards**: We'll start with US layout only

---

**Next: [Summary](summary.md)**
