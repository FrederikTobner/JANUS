# TinyOS Module Structure

## Module Organization Philosophy

**Flat hierarchy. Independent modules. No spaghetti.**

TinyOS follows a modular structure inspired by Linux and LLVM: each major component is a first-class module with clear boundaries and **zero circular dependencies**. If you create a circular dependency, you've designed it wrong. Fix it.

## Top-Level Directory Structure

```
TinyOS/
├── kernel/           # Core kernel functionality
│   ├── include/          # Global headers
│   ├── boot/             # Boot loader and initialization
│   ├── arch/             # Architecture-specific code
│   ├── lib/              # Utility libraries
│   ├── mm/               # Memory management
│   └── drivers/          # Device drivers (future)
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
│   └── bmunit/       # BMUnit testing framework
└── Documentation/    # Technical documentation
```

## Module Dependencies

Dependencies flow in **one direction**:

```
           ┌──────────┐
           │  core    │
           └────┬─────┘
                │
       ┌────────┴────────┐
       ▼                 ▼
  ┌─────────┐      ┌─────────┐
  │   mm    │      │  boot   │
  └────┬────┘      └────┬────┘
       │                │
       └────────┬───────┘
                ▼
          ┌──────────┐
          │  lib/X   │
          └────┬─────┘
               │
               ▼
      ┌──────────────────┐
      │  include/tinyos  │
      └──────────────────┘
```

If `buffer` needs something from `kernel`, you've layered it wrong. Move the functionality or redesign the interface.

### 3. Clear Layering

```
┌──────────────────────────────────────────────────────────────────────┐
│  Kernel Layer (kernel/core)                                          │
├──────────────────────────────────────────────────────────────────────┤
│  Subsystem Layer (kernel/mm/, kernel/drivers/, kernel/boot)          │
├──────────────────────────────────────────────────────────────────────┤
│  Library Layer (lib/*)                                               │
├──────────────────────────────────────────────────────────────────────┤
│  Architecture Layer (arch/x86_64/)                                   │
├──────────────────────────────────────────────────────────────────────┤
│  Boot Layer (boot/)                                                  │
└──────────────────────────────────────────────────────────────────────┘
```

## Core Modules

### `kernel/core/` - Core Kernel

Main kernel functionality and entry point.

**Responsibilities:**

- Kernel initialization sequence
- Main kernel loop
- Panic and error handling
- Global kernel state

### `boot/` - Boot Module

Boot loader interface and early initialization.

**Responsibilities:**

- Multiboot2 header and compliance
- Initial CPU state setup (stack, registers)
- Transition from boot loader to kernel

### `arch/` - Architecture Abstraction

Architecture-specific code with generic interfaces.

**Responsibilities:**

- Hardware I/O operations
- CPU-specific functions
- Platform initialization
- Hardware abstraction interfaces

## Library Modules

> Not fully defined yet.
