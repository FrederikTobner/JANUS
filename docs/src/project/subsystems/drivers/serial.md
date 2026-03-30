# Serial Driver

The serial driver provides character-level output to a UART port. On x86_64 it drives COM1 at I/O address `0x3F8`; on aarch64 it communicates with the PL011 UART. The baud rate is fixed at initialisation time.

Serial output is the first subsystem brought up during the init sequence, because it provides the earliest diagnostic channel available — it works before the framebuffer or VGA text mode are configured.

## Source Files

| File | Description |
|------|-------------|
| `include/drivers/serial.h` | Public API — `serial_init()`, `serial_putchar()`, `serial_write()` |
| `arch/include/arch/drivers/serial.h` | Architecture contract — each arch must implement `arch_serial_init()`, `arch_serial_putchar()` |
| `arch/x86_64/serial.c` | x86_64 implementation — COM1 via port I/O (`0x3F8`) |
| `arch/aarch64/serial.c` | aarch64 implementation — PL011 UART via MMIO |

## Initialisation

`kinit_serial()` is called from `kmain` as the first subsystem init step. It configures the UART hardware (baud rate, line control, FIFO) and leaves the port ready for character output.
