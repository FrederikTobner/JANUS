# Drivers

Hardware-facing device drivers subsystem.

## Current Drivers

### Serial (UART)

- Public API: `include/drivers/serial.h`
- Arch implementations: `arch/x86_64/serial.c`, `arch/aarch64/serial.c`
- Notes: COM1 (`0x3F8`) on x86_64, PL011 on aarch64. Currently only supports a fixed baud rate.

### Console (Text Terminal)

- Public API: `include/drivers/console.h`
- Arch contract: `arch/include/arch/drivers/console.h` (`console_ops_t`, `arch_console_probe`)
- Arch selectors: `arch/x86_64/console.c`, `arch/aarch64/console.c`
- Generic source: `src/console.c`
- Shared framebuffer backend: `arch/shared/fb_console.c`, `arch/shared/framebuffer.c`
- Notes: Supports VGA text mode (x86_64 Multiboot2) and framebuffer rendering
  (Limine on both architectures). No shadow buffer — device state lives in the backend.
