# TTY Driver

The TTY driver renders text to the screen. It supports two backends depending on the boot protocol and architecture:

- **VGA text mode** — used on x86_64 with Multiboot2, which does not provide a framebuffer. Writes directly to the VGA text buffer at `0xB8000`.
- **Framebuffer rendering** — used on Limine (both x86_64 and aarch64). Draws glyphs into the linear framebuffer provided by the bootloader.

## Source Files

| File | Description |
|------|-------------|
| `include/drivers/tty.h` | Public API — `tty_init()`, `tty_putchar()`, `tty_write()`, colour types |
| `arch/include/arch/drivers/tty.h` | Architecture contract — each arch must implement `arch_tty_init()`, `arch_tty_putchar()`, `arch_tty_set_color()` |
| `src/tty.c` | Shared logic — delegates to the architecture layer, handles newlines and scrolling at the generic level |
| `arch/x86_64/tty.c` | x86_64 implementation — dispatches between VGA text mode and framebuffer based on boot context |
| `arch/aarch64/tty.c` | aarch64 implementation — framebuffer rendering via PL011 |

## Framebuffer Renderer

A shared framebuffer renderer (`arch/shared/framebuffer.c`) handles the pixel-level glyph drawing and is linked by both architecture targets when framebuffer mode is active. It uses the Terminus font (`arch/shared/include/arch/shared/drivers/terminus.h`) to rasterise characters into the linear framebuffer.

## Display Mode Selection

During `kinit_tty()`, the init sequence checks whether the boot context contains a valid framebuffer. If a framebuffer is present, the TTY is initialised in framebuffer mode; otherwise it falls back to VGA text mode (x86_64 only, when the HHDM offset is zero — indicating a legacy BIOS boot without address translation).
