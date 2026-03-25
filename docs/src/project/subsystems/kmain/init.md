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

Initialises the TTY driver. Checks whether the boot context contains a valid framebuffer and selects the appropriate backend:

- **Framebuffer mode** — when `has_display` is true and a framebuffer address is available.
- **VGA text mode** — fallback on x86_64 when no framebuffer is present and the HHDM offset is zero (indicating a legacy BIOS boot).
- **No display** — if neither condition is met, TTY initialisation is skipped.
