# Drivers Subsystem

The drivers subsystem provides small, hardware-facing C APIs that the rest of the kernel uses to interact with devices. Each driver exposes a narrow public interface through `include/drivers/` and hides its architecture-specific implementation behind the three-tier include hierarchy.

## Drivers

| Driver | Description |
|--------|-------------|
| [TTY](tty.md) | Text terminal — VGA text mode or framebuffer rendering |
| [Serial](serial.md) | Character-level UART output (COM1 / PL011) |
| [CPU](cpu.md) | Processor control (halt, and eventually interrupt management) |

## Adding a New Driver

1. Add the public header to `include/drivers/`.
2. Add architecture-specific implementations under `arch/<arch>/`.
3. Add generic (architecture-independent) source to `src/` if applicable.
4. Keep the API narrow — avoid leaking hardware register layouts or protocol details through the public interface.
