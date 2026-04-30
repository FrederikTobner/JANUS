# Wiki

This wiki covers the foundational concepts that underpin operating system development. The material is general-purpose — nothing here is specific to JANUS — and a reader comfortable with C and basic computer architecture should be able to follow without prior OS experience.

Each article explains a single concept thoroughly enough to understand how it works and why it matters, without becoming a textbook. Where a concept has a direct application in JANUS, a side note links to the corresponding [Project](../project/introduction.md) page.

## Reading Order

The sections build on each other. Reading them in order is recommended.

1. **[Boot](boot/boot.md)** — What happens between power-on and the first line of kernel code. No prior OS knowledge assumed.
2. **[Memory](memory/memory.md)** — How the kernel discovers and manages RAM, and how virtual address spaces are constructed. Assumes familiarity with the boot process and the freestanding environment.
3. **[I/O](io/io.md)** — How the kernel communicates with hardware devices. Assumes understanding of memory-mapped regions from the Memory section.
4. **[Graphics](graphics/graphics.md)** — Display hardware from text-mode buffers to linear framebuffers. Assumes knowledge of I/O, MMIO, and the boot framebuffer provided by the bootloader.
