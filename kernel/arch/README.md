# TinyOS Architecture Layer

Architecture-specific implementations and hardware abstraction.
Only a stub for now.

## Supported Architectures

### x86_64

Primary target architecture for TinyOS.

**Contents:**

- `io.c` - Port I/O operations (inb, outb, etc.)
- `serial.c` - Serial port driver (COM1)
- `vga.c` - VGA text mode driver
- `cpu.c` - CPU-specific functions (CPUID, control registers, etc.)

**Public Interface:**
Headers in `x86_64/include/arch/`:

- `io.h` - I/O operations
- `serial.h` - Serial port interface
- `vga.h` - VGA interface
- `cpu.h` - CPU interface

## Architecture-Agnostic Interface

Headers in `include/arch/`:

- `types.h` - Architecture-independent type definitions

## Dependencies

- types (basic type definitions)
- buffer (for I/O formatting - future)
