# Drivers

Hardware-facing device drivers subsystem.

## Current Drivers

### Serial (UART)

- Public API: `include/drivers/serial.h`
- Arch implementations: `arch/x86_64/serial.c`, `arch/aarch64/serial.c`
- Notes: COM1 (`0x3F8`) on x86_64, PL011 on aarch64. Currently only supports a fixed baud rate.

### TTY (Text Terminal)

- Public API: `include/drivers/tty.h`
- Arch implementations: `arch/x86_64/tty.c`, `arch/aarch64/tty.c`
- Generic source: `src/tty.c`
- Shared framebuffer renderer: `arch/shared/framebuffer.c`
- Notes: Supports VGA text mode (x86_64 Multiboot2) and framebuffer rendering
  (Limine on both architectures).

