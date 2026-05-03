# Module Structure

JANUS follows a modular structure inspired by Linux and LLVM. Each major component is a first-class module with clear boundaries and zero circular dependencies.

## Directory Layout

```text
kernel/
├── include/          Global headers (janus/types.h, janus/attributes.h)
├── _start/           Entry point — creates kernel.elf
├── kmain/            Kernel main — composition root, links subsystems
├── lib/              Shared utility libraries
├── core/             Shared kernel services (e.g. kio)
└── subsys/           Independent subsystems
    ├── boot/             Boot protocol handling
    ├── drivers/          Device drivers
    └── mm/               Memory management
```

## Architecture Code Is Co-Located

Architecture-specific code lives inside the subsystem or library that needs it, not in a centralised `kernel/arch/` tree. This keeps a module's complete implementation — generic and platform-specific — together in one place, which simplifies navigation and reduces the risk of orphaned arch files when a module is refactored.

A typical subsystem with architecture code looks like:

```text
kernel/subsys/drivers/
├── include/drivers/                  Tier 1: public API
│   ├── cpu.h
│   ├── tty.h
│   └── serial.h
├── arch/
│   ├── include/arch/drivers/         Tier 2: contract headers
│   │   ├── cpu.h
│   │   ├── tty.h
│   │   └── serial.h
│   └── x86_64/
│       ├── include/arch/impl/drivers/  Tier 3: implementation
│       │   ├── cpu.h
│       │   └── io.h
│       ├── tty.c
│       └── serial.c
├── tty.c                             Generic driver code
└── serial.c
```

The three-tier include model is described in detail on the [Include Hierarchy](include-hierarchy.md) page.

## Module Summary

**`_start`** — The entry layer. Contains the assembly entry point, protocol header, and linker script. Organised by architecture and boot protocol (`x86_64/multiboot2/`, `x86_64/limine/`, `aarch64/limine/`). Build output: `kernel.elf` executable.

**`kmain`** — The composition root. Houses `kernel_main()`, the initialisation sequence, and the `kernel_descriptor_t` that threads boot-time data through to subsystem init functions. The only module permitted to link against subsystems. Build output: static library (linked by `_start`).

**`core`** — Shared kernel services above libraries and below subsystems. Core modules may depend on libraries, but must not depend on subsystems. Current module: `kio` (kernel formatted output and panic surface). Build output: static library.

**`boot`** — Parses boot protocol data and populates a `boot_context_t` with framebuffer info, memory map, and other firmware-provided state. Exposes `boot_init()` as its entry point. Build output: static library.

**`drivers`** — Device driver subsystem. Currently provides serial output (UART), TTY rendering (VGA text mode and framebuffer), and CPU control (`hlt`). Each driver has architecture-specific backends under `arch/`. Build output: static library.

**`mm`** — Memory management subsystem. Currently provides a bitmap-based PMM (physical frame allocator) and stats API. Virtual memory manager layers are planned on top. Build output: static library.

**`lib/`** — Freestanding utility libraries. `page_tables` provides architecture-specific page table manipulation. Future libraries include `mem_alloc` (heap allocator). Libraries in this layer have no knowledge of the subsystems above them. Build output: interface library (header-only) or static library, depending on module.
