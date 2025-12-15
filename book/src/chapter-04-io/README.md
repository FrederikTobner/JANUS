# Chapter 3: Input/Output

> *"Write programs to handle text streams, because that is a universal interface."
> — Doug McIlroy

A kernel that can't communicate is useless. We have a bootable kernel, but it's effectively blind and mute. No way to see what's happening, no way to debug, no way to interact with the user.

In this chapter, we'll give our kernel a voice and ears. We'll implement:

- **Serial I/O** — Output to COM1 serial port for debugging and logging
- **VGA Text Mode** — Display text on screen for user interaction
- **Keyboard Input** — Read keypresses and handle basic input

By the end of this chapter, our kernel will be able to print "Hello, TinyOS!" and respond to keyboard input. No more flying blind!

---

## Sections

1. [Serial Output](serial-output.md)
2. [VGA Text Mode](vga-text-mode.md)
3. [Keyboard Input](keyboard-input.md)
4. [Summary](summary.md)

---

**Next: [Serial Output](serial-output.md)**
