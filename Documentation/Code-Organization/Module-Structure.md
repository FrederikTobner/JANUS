# JANUS Module Structure

## Module Organization Philosophy

JANUS follows a modular structure inspired by Linux and LLVM: each major component is a first-class module with clear boundaries and **zero circular dependencies**. 

## Top-Level Directory Structure

```
janus/
├── kernel/           # Core kernel functionality
│   ├── include/          # Global headers (janus/types.h, etc.)
│   ├── _start/           # Entry point (creates kernel.elf)
│   ├── kmain/            # Kernel main - final assembly point
│   ├── lib/              # Shared utility libraries
│   └── subsys/           # Independent subsystems
│       ├── boot/             # Boot protocol handling
│       ├── drivers/          # Device drivers
│       └── mm/               # Memory management
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
└── Documentation/    # Technical documentation
```

### Architecture Code Lives Inside Subsystems

Architecture-specific code is **co-located with each subsystem** rather than in a separate `kernel/arch/` directory, like it is done in the Linux kernel.

```
kernel/subsys/drivers/
├── include/drivers/              # Public API (Tier 1)
│   ├── cpu.h                     # #include <drivers/cpu.h>
│   ├── tty.h
│   └── serial.h
├── arch/
│   ├── include/arch/drivers/     # Contract headers (Tier 2)
│   │   ├── cpu.h                 # #include <arch/drivers/cpu.h>
│   │   ├── tty.h
│   │   └── serial.h
│   └── x86_64/
│       ├── include/arch/impl/drivers/  # Implementation (Tier 3)
│       │   ├── cpu.h             # #include <arch/impl/drivers/cpu.h>
│       │   └── io.h
│       ├── tty.c                 # VGA text mode
│       └── serial.c              # COM1 UART
├── tty.c                         # Generic driver code
└── serial.c
```

## Module Layers

Modules are organized in layers. Dependencies flow **downward only**.

```
┌──────────────────────────────────────────────────────────────────────┐
│  Entry Layer (kernel/_start/)                                        │
│  - Creates kernel.elf executable                                     │
│  - Owns entry point (_start) and linker script                       │
├──────────────────────────────────────────────────────────────────────┤
│  Assembly Layer (kernel/kmain/)                                      │
│  - Final assembly point - links all subsystems                       │
│  - Kernel initialization (main.c)                                    │
│  - Only module allowed to depend on subsystems                       │
├──────────────────────────────────────────────────────────────────────┤
│  Subsystem Layer (kernel/subsys/*)                                   │
│  - boot/ handles protocol verification and boot info parsing         │
│  - mm/ handles memory management                                     │
│  - drivers/ provides device drivers                                  │
│  - Each subsystem contains its own arch/ code                        │
│  - Subsystems are ISOLATED: cannot depend on each other!             │
├──────────────────────────────────────────────────────────────────────┤
│  Support Library Layer (kernel/lib/)                                 │
│  - Freestanding utilities (hash, string, etc.)                       │
│  - Subsystems may depend on lib                                      │
├──────────────────────────────────────────────────────────────────────┤
│  Global Interface Layer (kernel/include/)                            │
│  - Type definitions, attributes, common headers                      │
└──────────────────────────────────────────────────────────────────────┘
```

## Module Dependency Flow

```
                    ┌──────────────┐
                    │   _start     │  ← Creates kernel.elf
                    └──────┬───────┘
                           │
                    ┌──────┴───────┐
                    │    kmain     │  ← Final assembly point
                    └──────┬───────┘    (only one that can link subsystems)
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
     ┌─────────┐    ┌─────────┐    ┌─────────┐
     │   boot  │    │    mm   │    │ drivers │   ← Each has its own arch/
     │  ┌───┐  │    │  ┌───┐  │    │  ┌───┐  │
     │  │x86│  │    │  │x86│  │    │  │x86│  │
     │  └───┘  │    │  └───┘  │    │  └───┘  │
     └────┬────┘    └────┬────┘    └────┬────┘
          │              │              │
          └──────────────┼──────────────┘
                         │
                    ┌────┴─────┐
                    │  lib/*   │  ← Can have arch/ too
                    └────┬─────┘
                         │
                         ▼
                ┌──────────────────┐
                │  include/janus   │
                └──────────────────┘
```

**Key insight:**

- `_start/` creates `kernel.elf` by linking `kmain` and all subsystems
- `kmain/` is the **only** module that can depend on subsystems (enforced by CMake!)
- Subsystems are **isolated**: they cannot depend on each other
- Each subsystem contains its own `arch/` folder with architecture-specific code

## Module Descriptions

### `kernel/_start/` - Entry Point Module

Entry point and final executable creation. Organized by `${ARCH}/${PROTOCOL}`.

**Responsibilities:**

- Entry point (`_start`) in assembly
- Protocol header (Multiboot2, Limine, etc.)
- Linker script (memory layout)
- **Creates `kernel.elf`** by linking all other modules

**Structure:**

```
kernel/_start/
├── CMakeLists.txt              # Dispatches to arch/protocol
└── x86_64/
    └── multiboot2/
        ├── CMakeLists.txt      # Creates kernel.elf
        ├── link.ld             # Linker script
        ├── boot.asm            # Entry point (_start)
        └── multiboot2.asm      # Protocol header
```

**Build output:** `kernel.elf` executable

### `kernel/kmain/` - Kernel Main (Assembly Point)

The final assembly point that links all subsystems together.

**Responsibilities:**

- Kernel initialization sequence (`kernel_main()`)
- Main kernel loop
- Panic and error handling
- **Only module allowed to depend on subsystems** (enforced by CMake!)

**Build output:** Static library (linked by _start)

### `kernel/subsys/boot/` - Boot Protocol Module

Boot protocol handling and verification.

**Responsibilities:**

- Boot handoff verification
- Boot info structure parsing
- Protocol-specific definitions

**Build output:** Static library

### `kernel/subsys/drivers/` - Device Drivers

Hardware device drivers with architecture-specific implementations.

**Responsibilities:**

- VGA/framebuffer output
- Serial console
- CPU operations (halt, interrupts)
- Future: disk, network, etc.

**Structure (with arch):**

```
kernel/subsys/drivers/
├── include/drivers/              # Tier 1: Public API
│   ├── cpu.h                     # #include <drivers/cpu.h>
│   ├── tty.h
│   └── serial.h
├── arch/
│   ├── include/arch/drivers/     # Tier 2: Contract headers
│   │   ├── cpu.h                 # #include <arch/drivers/cpu.h>
│   │   ├── tty.h
│   │   └── serial.h
│   └── x86_64/
│       ├── include/arch/impl/drivers/  # Tier 3: Implementation
│       │   ├── cpu.h             # Header-only (inline asm)
│       │   └── io.h              # Port I/O helpers
│       ├── tty.c                 # VGA text mode
│       └── serial.c              # COM1 UART
├── tty.c                         # Generic driver code
└── serial.c
```

**Build output:** Static library

### `kernel/subsys/mm/` - Memory Management

Memory management subsystem.

**Responsibilities:**

- Physical memory allocator
- Virtual memory management
- Page tables

**Build output:** Static library (placeholder)

### `kernel/lib/` - Support Libraries

Freestanding utility libraries with no kernel dependencies.

**Responsibilities:**

- String manipulation
- Hash functions (FNV-1a, etc.)
- Data structures

**Build output:** Interface library (header-only) or static library

## Three-Tier Include Model

Each subsystem with architecture code uses a three-tier include hierarchy:

```
┌─────────────────────────────────────────────────────────────────────┐
│  Tier 1: Public API (subsys/foo/include/foo/)                       │
│  #include <drivers/cpu.h>                                           │
│  - What consumers use                                               │
│  - Architecture-agnostic interface                                  │
│  - Includes Tier 2 contract                                         │
├─────────────────────────────────────────────────────────────────────┤
│  Tier 2: Contract Headers (subsys/foo/arch/include/arch/foo/)       │
│  #include <arch/drivers/cpu.h>                                      │
│  - Bridge between public and impl                                   │
│  - Declares arch_* functions                                        │
│  - Includes Tier 3 for current architecture                         │
├─────────────────────────────────────────────────────────────────────┤
│  Tier 3: Implementation (subsys/foo/arch/<ARCH>/include/arch/impl/) │
│  #include <arch/impl/drivers/cpu.h>                                 │
│  - x86_64, aarch64, etc.                                            │
│  - Actual inline assembly or .c files                               │
│  - Never included directly by consumers                             │
└─────────────────────────────────────────────────────────────────────┘
```

## Architecture Layer Structure

For a detailed explanation of the architecture (arch) layer patterns, directory conventions, and best practices—including how to organize arch code in subsystems and libraries—see [Arch-Layer-Structure.md](./Arch-Layer-Structure.md).
