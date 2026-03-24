# Drivers Subsystem

The drivers subsystem provides small, hardware-facing C APIs that the rest of the kernel uses to interact with devices. Each driver exposes a narrow public interface through `include/drivers/` and hides its architecture-specific implementation behind the three-tier include hierarchy.

## Current Drivers

### Serial (UART)

The serial driver provides character-level output to a UART port. On x86_64 it drives COM1 at I/O address `0x3F8`; on aarch64 it communicates with the PL011 UART. The baud rate is fixed at initialisation time.

Serial output is the first subsystem brought up during the init sequence, because it provides the earliest diagnostic channel available — it works before the framebuffer or VGA text mode are configured.

### TTY (Text Terminal)

The TTY driver renders text to the screen. It supports two backends depending on the boot protocol and architecture:

- **VGA text mode** — used on x86_64 with Multiboot2, which does not provide a framebuffer. Writes directly to the VGA text buffer at `0xB8000`.
- **Framebuffer rendering** — used on Limine (both x86_64 and aarch64). Draws glyphs into the linear framebuffer provided by the bootloader.

A shared framebuffer renderer (`arch/shared/framebuffer.c`) handles the pixel-level glyph drawing and is linked by both architecture targets when framebuffer mode is active.

### CPU

The CPU driver currently provides a single function: `drivers_cpu_halt_forever()`, which halts the processor in an infinite loop. As the kernel grows, this module will expand to cover interrupt control and processor state management.

## Adding a New Driver

1. Add the public header to `include/drivers/`.
2. Add architecture-specific implementations under `arch/<arch>/`.
3. Add generic (architecture-independent) source to `src/` if applicable.
4. Keep the API narrow — avoid leaking hardware register layouts or protocol details through the public interface.
