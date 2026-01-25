# drivers - Device Drivers Module

Hardware-facing drivers that provide small, C-friendly APIs to the rest of the kernel.

The overall structure is meant to stay simple and be easy to extend: each driver lives in its own folder, and its public interface lives under `include/drivers/`.

## Current drivers

### UART (COM1)

- Implementation: `uart/uart.c`
- Public API: `include/drivers/uart.h`
- Notes: currently targets COM1 (`0x3F8`) and uses a fixed baud rate (see header/source for details).

### VGA text mode

- Implementation: `video/vga_text.c`
- Public API: `include/drivers/vga_text.h`
- Notes: basic 80x25 text output and scrolling.

## Directory layout

```text
drivers/
├── CMakeLists.txt
├── README.md
├── include/
│   └── drivers/
│       ├── uart.h
│       └── vga_text.h
├── uart/
│   └── uart.c
└── video/
      └── vga_text.c
```

## Build integration

`drivers/CMakeLists.txt` glob-collects all `*.c` files under this module and builds them into the `drivers` library.

Note: because this uses `file(GLOB_RECURSE ...)`, adding new source files may require re-running CMake configuration (e.g. `cmake -S . -B build`) so the new files are picked up.

## Dependencies

- `arch` (currently for port I/O via `arch/io.h`, used by the UART driver)
- `tinyos` base headers (e.g. `tinyos/types.h`)

## Adding a new driver

Minimal convention:

1. Create a folder (e.g. `storage/ata.c`).
2. Add its public header to `include/drivers/` (e.g. `include/drivers/ata.h`).
3. Keep the API narrow and avoid leaking architecture/protocol details.
