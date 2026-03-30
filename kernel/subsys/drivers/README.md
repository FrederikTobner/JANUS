# drivers — Device Drivers Subsystem

Hardware-facing drivers that provide small, C-friendly APIs to the rest of the
kernel.

## Current Drivers

### Serial (UART)

- Public API: `include/drivers/serial.h`
- Arch implementations: `arch/x86_64/serial.c`, `arch/aarch64/serial.c`
- Notes: COM1 (`0x3F8`) on x86_64, PL011 on aarch64. Fixed baud rate.

### TTY (Text Terminal)

- Public API: `include/drivers/tty.h`
- Arch implementations: `arch/x86_64/tty.c`, `arch/aarch64/tty.c`
- Generic source: `src/tty.c`
- Shared framebuffer renderer: `arch/shared/framebuffer.c`
- Notes: Supports VGA text mode (x86_64 Multiboot2) and framebuffer rendering
  (Limine on both architectures).

### CPU

- Public API: `include/drivers/cpu.h`
- Notes: Provides `drivers_cpu_halt_forever()` for fatal halts.

## Directory Layout

```text
drivers/
├── CMakeLists.txt
├── README.md
├── include/
│   └── drivers/
│       ├── cpu.h
│       ├── serial.h
│       └── tty.h
├── src/
│   └── tty.c                  # Generic TTY logic
└── arch/
    ├── include/
    │   └── arch/
    │       └── drivers/        # Tier 2 contract headers
    ├── shared/
    │   ├── include/
    │   │   └── arch/
    │   │       └── impl/
    │   │           └── drivers/
    │   └── framebuffer.c       # Shared framebuffer renderer
    ├── x86_64/
    │   ├── include/
    │   │   └── arch/
    │   │       └── impl/
    │   │           └── drivers/
    │   ├── internal/
    │   ├── serial.c
    │   └── tty.c
    └── aarch64/
        ├── include/
        │   └── arch/
        │       └── impl/
        │           └── drivers/
        ├── internal/
        ├── serial.c
        └── tty.c
```

## Build Integration

Uses `janus_add_subsys(drivers ...)` which automatically:
- Detects the `arch/` folder and globs architecture-specific sources
- Sets up three-tier include paths (Tier 1: public, Tier 2: arch contract, Tier 3: arch impl)
- Enforces subsystem isolation (drivers cannot depend on other subsystems)

## Adding a New Driver

1. Add the public header to `include/drivers/` (e.g., `include/drivers/keyboard.h`)
2. Add architecture-specific implementations under `arch/<arch>/`
3. Add generic (arch-independent) source to `src/` if applicable
4. Keep the API narrow — avoid leaking hardware/protocol details
