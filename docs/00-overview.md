# JANUS — Overview

JANUS is a freestanding kernel written in C17, targeting x86_64 and aarch64.

## Design Principles

### Strict Layering

JANUS is organised into layers with one-directional dependencies. No module may
depend on anything in the same layer or above it; violations are caught at CMake
configure time. This discipline keeps each layer independently comprehensible and
eliminates entire classes of coupling bugs before they can reach the linker.

```mermaid
graph TD
  subgraph entry["_start — Entry"]
    _start
  end
  subgraph composition["kmain — Composition root"]
    kmain
  end
  subgraph subsys["Subsystems"]
    boot
    drivers
    mm
  end
  subgraph core["Core services"]
    kio
  end
  subgraph libs["Libraries"]
    fmt
    display
    page_tables
  end
  subgraph asm_layer["ASM abstraction"]
    janus_asm
  end
  subgraph global["Global headers"]
    types_h["types.h"]
    attributes_h["attributes.h"]
    config_h["config.h"]
  end
  _start --> kmain
  kmain --> boot
  kmain --> drivers
  kmain --> mm
  kmain --> kio
  boot --> kio
  drivers --> kio
  mm --> kio
  kio --> fmt
  drivers --> display
  drivers --> janus_asm
  mm --> janus_asm
  fmt --> types_h
  janus_asm --> types_h
```

### Modular, Co-located Architecture

Each module has clear boundaries, explicit dependencies, and a public header
interface that is the sole point of contact with the rest of the kernel.
Rather
than centralising all platform code in a single `arch/` tree, every subsystem and
library contains its own `arch/` subdirectory.
This means a module's complete implementation, generic logic and platform-specific code alike, is navigable as a single unit without jumping between distant directories.

### Public Structures Over Opaque Handles

Kernel data structures are defined publicly rather than hidden behind opaque
pointer typedefs.
In a freestanding environment, the canonical justification for opaque handles, binary API stability across shared-library boundaries, simply does not exist, while the costs are immediate and tangible: pointer indirection, mandatory heap allocation, degraded cache locality, and structures that appear as featureless `void *` values in a debugger.
Public structures admit stack allocation, in-place embedding for cache-friendly layouts, and complete visibility in LLDB or
GDB.
Opaque handles are reserved for the narrow set of genuine abstraction boundaries where the underlying representation varies at runtime, such as framebuffer backends.

## Compiler Support

Both GCC and Clang are supported and tested in CI across the full
architecture matrix:

| Preset          | Compiler | Architecture |
|-----------------|----------|--------------|
| `x86_64-gcc`    | GCC      | x86_64       |
| `x86_64-clang`  | Clang    | x86_64       |
| `aarch64-gcc`   | GCC      | aarch64      |
| `aarch64-clang` | Clang    | aarch64      |

All configurations compile with `-Wall -Wextra -Werror -Wconversion` combined with the freestanding flags `-ffreestanding -fno-builtin -nostdlib`.
