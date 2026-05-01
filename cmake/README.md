# cmake — CMake Build Helpers

CMake modules for cross-compilation toolchains, platform detection, and
kernel build helpers.

## Directory Layout

```
cmake/
├── toolchains/          # Toolchain files (set compiler + binutils)
├── arch/                # Per-architecture compiler flags
├── platform/            # Platform detection and common compile flags
├── boot/                # Boot protocol cmake helpers
├── kernel/              # Kernel target helpers
└── Registry.cmake       # Dependency registry + Mermaid graph generator
```

## toolchains/

Toolchain files processed before `project()`. Sets the compiler, linker, and
binutils for each architecture/compiler combination.

| File | Target |
|---|---|
| `x86_64-gcc.cmake` | GCC, x86\_64 |
| `x86_64-clang.cmake` | Clang, x86\_64 |
| `aarch64-gcc.cmake` | GCC cross-compiler, AArch64 |
| `aarch64-clang.cmake` | Clang cross-compiler, AArch64 |

## platform/

`Detection.cmake` — host detection, compiler validation, freestanding flags
(`-ffreestanding`, `-nostdlib`, warnings), and build-type flags. Includes
`arch/<arch>/JanusPlatform.cmake` for ISA-specific flags.

## kernel/

Helper functions for adding kernel targets.

### `Library.cmake` — `janus_add_library(name SOURCES ... [DEPENDENCIES ...])`

Creates a kernel library (STATIC, or INTERFACE when no sources are provided).
Adds `include/` and global `kernel/include/` to include paths, links `janus_asm`,
and registers the target in the dependency registry.

### `Subsystem.cmake` — `janus_add_subsys(name SOURCES ... [DEPENDENCIES ...])`

Creates a kernel subsystem. Automatically detects an `arch/CMakeLists.txt` and
calls `add_subdirectory(arch)`. **Sources must be listed explicitly** — no
globbing. Enforces subsystem isolation: a fatal error is raised if a subsystem
depends on another subsystem (except `kmain`).

### `ArchSource.cmake` — `janus_add_arch_subsys(name SOURCES ...)`

Called from within `arch/CMakeLists.txt`. Creates a `${name}_arch` static
library with the three-tier include hierarchy as PUBLIC paths. Links `janus_asm`
so Tier 3 headers can use `#include <asm/cpu.h>` etc.

### `Executable.cmake` — `janus_add_kernel(TARGET ... LINKER_SCRIPT ... DEPENDENCIES ... OBJECTS ...)`

Links a kernel ELF from object libraries and dependencies. Called from
`_start/<arch>/` to produce the final `kernel-<protocol>.elf`.

## Registry.cmake

Tracks all registered targets. Called after all `add_subdirectory` calls.

- `janus_validate_registry()` — verify isolation rules
- `janus_write_mermaid_diagram(output_file)` — write a Mermaid dependency
  graph to `docs/src/generated/deps-<arch>.md`

**Node shapes:** LIB → rounded rectangle, SUBSYS → rectangle, ASM → cylinder,
PROTOCOL\_LIB → subroutine box, EXEC → hexagon.
