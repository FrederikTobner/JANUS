# JANUS Global Include Directory

## Purpose

The [`include/janus/`](../../include/janus/) directory contains **truly global** headers that are used across many modules and don't belong to any single module.

> **Rule**: If a header is only used by a couple of kernel modules, keep it in a kernel library. Only promote to global if it's genuinely cross-cutting.

## What Goes in `kernel/include/janus/`

### Core Kernel Definitions

Headers that define fundamental kernel-wide concepts:

- **`types.h`** - Global type aliases and kernel-specific types
  - Fixed-width integer types (u8, s8, u16, u32, u64)

- **`kernel.h`** - Core kernel macros and constants

- **`compiler.h`** - Compiler attributes

### Kernel Utility Libraries

For functionality used by many kernel modules we are using the `/kernel/lib` directory.
Kernel utility libraries are isolated from each other and are not allowed to depend on any other kernel libraries or kernel modules.
They are only allowed to use our core kernel definitions from `kernel/include/janus/`.

### Kernel Subsystems

Subsystems live in `kernel/subsys/` and are **isolated building blocks**:

- `boot/` - Boot protocol handling
- `drivers/` - Device drivers  
- `mm/` - Memory management

**Subsystems cannot depend on each other!** This is enforced by CMake at configure time.

### Architecture Code Inside Subsystems

Each subsystem can have its own `arch/` folder with architecture-specific code:

```
kernel/subsys/drivers/
├── include/drivers/          # Public API (Tier 1)
│   └── cpu.h                 # #include <drivers/cpu.h>
├── arch/
│   ├── include/arch/drivers/ # Contract (Tier 2)
│   │   └── cpu.h             # #include <arch/drivers/cpu.h>
│   └── x86_64/
│       └── include/arch/impl/drivers/  # Implementation (Tier 3)
│           └── cpu.h         # #include <arch/impl/drivers/cpu.h>
```

Include paths are namespaced by subsystem (`arch/drivers/`, not `arch/`) to prevent collisions when multiple subsystems have arch code.

### Kernel Assembly Point (kmain)

This is the only module allowed to depend on subsystems.
It is the entry point of the kernel and is responsible for initializing and coordinating all subsystems.
It lives in `kernel/kmain/`.

### User API Definitions

> **TODO:** Add user API definition once it is ready.
