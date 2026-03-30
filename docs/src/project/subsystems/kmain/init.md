# Init Sequence

The init module contains the individual subsystem initialisation functions called by `kernel_main()`. Each function takes a pointer to the `boot_context_t` and configures a single subsystem.

## Source Files

| File | Description |
|------|-------------|
| `src/init.c` | `kinit_serial()`, `kinit_tty()` implementations |
| `internal/kmain/init.h` | Internal prototypes for init functions |

## Functions

### `kinit_serial`

Initialises the serial driver using architecture-specific UART configuration. This is always the first subsystem brought up, because serial output provides the earliest diagnostic channel.

### `kinit_tty`

Initialises the TTY driver. Dispatches on the `display_mode` field of the boot context:

- **`BOOT_DISPLAY_FRAMEBUFFER`** — a linear RGB framebuffer is available (Limine, or Multiboot2 with a framebuffer header tag). Initialises the framebuffer text renderer.
- **`BOOT_DISPLAY_VGA_TEXT`** — the bootloader confirmed VGA text hardware is present (Multiboot2 EGA text mode on x86_64). Initialises VGA text mode at `0xB8000`.
- **`BOOT_DISPLAY_NONE`** — no display available. TTY initialisation is skipped; serial remains the only output channel.
