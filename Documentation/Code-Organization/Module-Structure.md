# JANUS Module Structure

## Module Organization Philosophy

**Flat hierarchy. Independent modules. No spaghetti.**

JANUS follows a modular structure inspired by Linux and LLVM: each major component is a first-class module with clear boundaries and **zero circular dependencies**. If you create a circular dependency, you've designed it wrong. Fix it.

## Top-Level Directory Structure

```
janus/
├── kernel/           # Core kernel functionality
│   ├── include/          # Global headers
│   ├── entry/            # Entry points (creates kernel.elf)
│   ├── init/             # Kernel initialization (main.c)
│   ├── boot/             # Boot protocol handling
│   ├── arch/             # Architecture-specific code
│   ├── lib/              # Utility libraries
│   ├── mm/               # Memory management
│   └── drivers/          # Device drivers
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
│   └── bmunit/       # BMUnit testing framework
└── Documentation/    # Technical documentation
```

## Module Layers

Modules are organized in layers. Dependencies flow **downward only**.

```
┌──────────────────────────────────────────────────────────────────────┐
│  Entry Layer (kernel/entry/)                                         │
│  - Creates kernel.elf executable                                     │
│  - Owns entry point (_start) and linker script                       │
├──────────────────────────────────────────────────────────────────────┤
│  Init Layer (kernel/init/)                                           │
│  - Kernel initialization (main.c)                                    │
│  - Protocol-agnostic, architecture-agnostic                          │
├──────────────────────────────────────────────────────────────────────┤
│  Subsystem Layer (kernel/mm/, kernel/drivers/, kernel/boot/)         │
│  - boot/ handles protocol verification and boot info parsing         │
│  - mm/ handles memory management                                     │
│  - drivers/ provides device drivers                                  │
├──────────────────────────────────────────────────────────────────────┤
│  Support Library Layer (kernel/lib/)                                 │
│  - Freestanding utilities (hash, string, etc.)                       │
├──────────────────────────────────────────────────────────────────────┤
│  Platform/Architecture Layer (kernel/arch/)                          │
│  - CPU-specific code (x86_64, aarch64, etc.)                         │
├──────────────────────────────────────────────────────────────────────┤
│  Global Interface Layer (kernel/include/)                            │
│  - Type definitions, attributes, common headers                      │
└──────────────────────────────────────────────────────────────────────┘
```

## Module Dependency Flow

```
                    ┌──────────────┐
                    │    entry     │  ← Creates kernel.elf
                    └──────┬───────┘
                           │
                    ┌──────┴───────┐
                    │     init     │  ← Protocol-agnostic init
                    └──────┬───────┘
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
    ┌──────────┐    ┌──────────┐    ┌──────────┐
    │   boot   │    │    mm    │    │ drivers  │
    │(protocol │    │          │    │          │
    │  verify) │    │          │    │          │
    └────┬─────┘    └────┬─────┘    └────┬─────┘
         │               │               │
         └───────────────┼───────────────┘
                         ▼
                  ┌─────────────┐
                  │  arch/x86   │
                  └──────┬──────┘
                         │
                         ▼
                    ┌──────────┐
                    │  lib/*   │
                    └────┬─────┘
                         │
                         ▼
                ┌──────────────────┐
                │  include/janus   │
                └──────────────────┘
```

**Key insight:** `entry/` is responsible for linking all modules into the final `kernel.elf`. It owns the entry point (`_start`) and the linker script. `boot/` provides protocol verification as a library.

## Module Descriptions

### `kernel/entry/` - Entry Point Module

Entry point and final executable creation. Organized by `${ARCH}/${PROTOCOL}`.

**Responsibilities:**

- Entry point (`_start`) in assembly
- Protocol header (Multiboot2, Limine, etc.)
- Linker script (memory layout)
- **Creates `kernel.elf`** by linking all other modules

**Structure:**

```
kernel/entry/
├── CMakeLists.txt              # Dispatches to arch/protocol
└── x86_64/
    └── multiboot2/
        ├── CMakeLists.txt      # Creates kernel.elf
        ├── link.ld             # Linker script
        ├── boot.asm            # Entry point (_start)
        └── multiboot2.asm      # Protocol header
```

**Build output:** `kernel.elf` executable

### `kernel/init/` - Kernel Initialization

Protocol-agnostic and architecture-agnostic kernel initialization.

**Responsibilities:**

- Kernel initialization sequence (`kernel_main()`)
- Main kernel loop
- Panic and error handling
- Global kernel state

**Build output:** Object library (linked by entry)

### `kernel/boot/` - Boot Protocol Module

Boot protocol handling and verification. Organized by `${ARCH}/${PROTOCOL}`.

**Responsibilities:**

- Boot handoff verification
- Boot info structure parsing
- Protocol-specific definitions

**Structure:**

```
kernel/boot/
├── CMakeLists.txt              # Dispatches to arch/protocol
├── include/                    # Shared boot headers
└── x86_64/
    └── multiboot2/
        ├── CMakeLists.txt      # Creates static library
        ├── multiboot2.h        # Protocol definitions
        └── verify.c            # Handoff verification
```

**Build output:** Static library

### `kernel/arch/` - Architecture Abstraction

Architecture-specific code with generic interfaces.

**Responsibilities:**

- Hardware I/O operations
- CPU-specific functions
- Platform initialization
- Hardware abstraction interfaces

**Build output:** Static library

### `kernel/drivers/` - Device Drivers

Hardware device drivers.

**Responsibilities:**

- VGA/framebuffer output
- Serial console
- Keyboard input
- Future: disk, network, etc.

**Build output:** Static library

### `kernel/lib/` - Support Libraries

Freestanding utility libraries with no kernel dependencies.

**Responsibilities:**

- String manipulation
- Hash functions (FNV-1a, etc.)
- Data structures

**Build output:** Interface library (header-only) or static library

## Build Flow

```
┌──────────────────────────────────────────────────────────────────┐
│                    kernel/CMakeLists.txt                         │
│  add_subdirectory(lib)      # Built first (utilities)            │
│  add_subdirectory(arch)     # Architecture support               │
│  add_subdirectory(drivers)  # Device drivers                     │
│  add_subdirectory(boot)     # Protocol verification library      │
│  add_subdirectory(init)     # Kernel initialization              │
│  add_subdirectory(entry)    # Built last, creates kernel.elf     │
└──────────────────────────────────────────────────────────────────┘
                           │
    ┌──────────────────────┼──────────────────────┬────────────────┐
    ▼                      ▼                      ▼                ▼
┌──────────┐        ┌──────────┐          ┌──────────┐      ┌──────────┐
│   init   │        │   arch   │          │ drivers  │      │   lib    │
│ (OBJECT) │        │ (STATIC) │          │ (STATIC) │      │(INTERFACE│
└────┬─────┘        └────┬─────┘          └────┬─────┘      └──────────┘
     │                   │                     │
     │              ┌────┴─────┐               │
     │              │   boot   │               │
     │              │ (STATIC) │               │
     │              └────┬─────┘               │
     │                   │                     │
     └───────────────────┼─────────────────────┘
                         ▼
          ┌──────────────────────────────────┐
          │  entry/${ARCH}/${PROTOCOL}/      │
          │  janus_link_kernel(...)          │
          └────────────────┬─────────────────┘
                           ▼
                    ┌──────────────┐
                    │  kernel.elf  │
                    └──────────────┘
```
