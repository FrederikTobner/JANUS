# Kernel Root Directory

Contains all the code needed to create the JANUS kernel.
The kernel is built as a freestanding executable (`kernel.elf`) that can be booted by Limine or GRUB (Multiboot2).

## Sub Folders

### cmake/

Kernel-only CMake helper functions (`janus_add_library`, `janus_add_core`, `janus_add_subsys`, ...) and kernel ABI compiler flags. Only ever included from `kernel/CMakeLists.txt`. See [cmake/README.md](./cmake/README.md) for more details.

### asm

Inline assembly layer. The **only** permitted location for `__asm__ volatile` in kernel C code.
See [asm/README.md](asm/README.md) for more details

### _start/

Entry point module that creates `kernel.elf` by linking all subsystems.
Organized by architecture and boot protocol.
Contains assembly entry points, linker scripts, and protocol-specific request structures.
See [_start/README.md](_start/README.md) for more details

### kmain/

Kernel initialization and composition root.
Owns `kernel_main()`, which acts as the architecture and boot protocol agnostic main entry point of the kernel.
See [kmain/README.md](kmain/README.md) for more details.

### lib/

Freestanding utility libraries shared across subsystems
See [lib/README.md](./lib/README.md) for more details.

### core/

Shared kernel core services used by multiple subsystems without introducing subsystem-to-subsystem dependencies.
See [core/README.md](core/README.md)for more details.

### subsys/

The main kernel subsystems (`boot`, `drivers`, `mm`).
Each subsystem owns its hardware state and exposes a public API.
Subsystems are isolated from each other — only `kmain` may depend on multiple subsystems.
See [subsys/README.md](subsys/README.md)for more details.

### include/janus/

Global kernel headers: `types.h`, `attributes.h`, `config.h` (generated).
These have no dependencies and are visible everywhere.
See [include/README.md](include/README.md)for more details.

### tests

Tests of the kernel.
See [tests/README.md](tests/README.md) for more details.

## Layer Order

```
_start → kmain → subsys → core → lib → asm → include
```

Each layer may only depend on layers lower in the stack.
`kmain` is the sole subsystem that is permitted to combine multiple subsystems.
