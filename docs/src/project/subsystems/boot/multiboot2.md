# Multiboot2 Protocol

The Multiboot2 boot protocol implementation populates the `boot_context_t` by parsing the Multiboot2 information structure — a contiguous block of tagged data passed by the bootloader.

## Source Files

| File | Description |
|------|-------------|
| `protocol/multiboot2/multiboot2_boot.c` | `boot_init()` implementation — walks the tag list |
| `protocol/multiboot2/multiboot2_protocol.h` | Multiboot2 tag type definitions and constants |

## How It Works

The bootloader passes two values in registers: a magic number (`0x36D76289`) and a pointer to the information structure. The assembly entry point stashes these before setting up paging and calling `kernel_main`.

`multiboot2_boot.c` walks the tag list sequentially, handling:

- **Tag type 8 (framebuffer)** — extracts framebuffer address, dimensions, pitch, bpp, and RGB colour field positions. Only RGB framebuffers (`fb_type == 1`) are accepted.
- **Tag type 6 (memory map)** — provides the physical memory layout.

## Assembly Entry

The Multiboot2 entry point has significantly more work to do than Limine's:

1. Stash the magic/info pointer from registers.
2. Set up identity-mapped page tables (4 GB to cover the framebuffer).
3. Enable PAE, long mode, and paging.
4. Load a 64-bit GDT and far-jump to long mode.
5. Call `kernel_main`.

See the [Mode Switch tutorial](../../../tutorials/boot/x86-64-mode-switch.md) for a detailed walkthrough.
