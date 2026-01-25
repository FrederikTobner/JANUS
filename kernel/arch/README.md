# TinyOS Architecture Layer

Architecture-specific code and small hardware abstractions.

The goal of this module is to keep the rest of the kernel (core, drivers, mm, …) as architecture-independent as possible by providing narrow, stable APIs.

## Supported architectures

### x86_64

This is currently the only supported target architecture.

## Public interface

Public headers live under `include/arch/` and are intended to be usable from any kernel module:

- `io.h` — port I/O helpers (used e.g. by UART)
- `cpu.h` — CPU control helpers (halt / halt forever)

## Current implementation

x86_64-specific implementations live under `x86_64/`:

- `x86_64/io.c` — implementation of `outb()` / `inb()`
- `x86_64/cpu.c` — implementation of `cpu_halt()` / `cpu_halt_forever()`

## Directory layout

```text
arch/
├── CMakeLists.txt
├── README.md
├── include/
│   └── arch/
│       ├── cpu.h
│       └── io.h
└── x86_64/
    ├── cpu.c
    └── io.c
```

## Build integration

`arch/CMakeLists.txt` selects sources from `arch/${TINYOS_TARGET_ARCH}/` and always exports the public headers from `arch/include/arch/`.

Note: because it uses `file(GLOB_RECURSE ...)`, adding new source files may require re-running CMake configuration (e.g. `cmake -S . -B build`) so the new files are picked up.

## Adding a new architecture or feature

Keep it minimal and extendable:

1. Add a small, architecture-neutral header in `include/arch/`.
2. Implement it in `arch/<arch>/`.
3. Avoid leaking instruction set details to other modules; prefer APIs like `cpu_halt_forever()` over embedding inline assembly in core code.

