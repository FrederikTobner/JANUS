# Graphics

This section covers the two display output mechanisms a kernel is likely to encounter: the legacy VGA text mode memory region and the linear framebuffer provided by modern boot protocols. Both allow text output from early kernel code without a full graphics stack.

**Prerequisites:** [Boot](../boot/boot.md) and [I/O](../io/io.md). The framebuffer base address and dimensions come from the bootloader, and framebuffer writes are a form of MMIO — understanding both is assumed.

## Articles

1. **[VGA Text Mode](vga-text-mode.md)** — The legacy 80×25 character grid mapped at `0xB8000`: cell layout, colour attributes, scrolling, and its x86-only limitation.
2. **[Framebuffers](framebuffers.md)** — Linear pixel buffers: address calculation, pitch vs width, 32-bit BGRA colour format, and how a kernel renders text into pixel memory using rasterised glyph bitmaps.
