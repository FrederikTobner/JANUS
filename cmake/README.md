# cmake — CMake Build Helpers

CMake modules for cross-compilation toolchains, platform detection, and kernel build helpers.

## Directory Layout

```
cmake/
├── toolchains/          # Toolchain files (set compiler + binutils)
├── arch/                # Per-architecture compiler flags
├── platform/            # Platform detection and common compile flags
├── boot/                # Boot protocol cmake helpers
├── kernel/              # Kernel target helpers
├── Registry.cmake       # Dependency registry + Mermaid graph generator
└── Targets.cmake        # Defines all the phony targets of the kernel
```

## platform/

`Detection.cmake` is used for host detection, compiler validation, freestanding flags (`-ffreestanding`, `-nostdlib`, warnings), and build-type flags. It includes `arch/<arch>/JanusPlatform.cmake` for ISA-specific flags.

## kernel/

This folder contains helper functions for adding kernel targets.

### `Library.cmake` — `janus_add_library(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a kernel library (STATIC, or INTERFACE when no sources are provided).

### `Core.cmake`- `janus_add_core(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a core service.

### `Contract.cmake` - `janus_add_contract(name [CONSUMERS ...])`

Used for creating a contract between some subsystems, that need to share a datastructure.

### `Subsystem.cmake` — `janus_add_subsys(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a kernel subsystem.
Automatically detects an `arch/CMakeLists.txt` and calls `add_subdirectory(arch)`.

### `ArchSource.cmake` — `janus_add_arch_subsys(name SOURCES ...)`

Called from within `arch/CMakeLists.txt`.
Creates a `${name}_arch` static library with the three-tier include hierarchy as PUBLIC paths.

### `Executable.cmake` — `janus_add_kernel(TARGET ... LINKER_SCRIPT ... DEPENDENCIES ... OBJECTS ...)`

Links a kernel ELF from object libraries and dependencies.
Called from `_start/<arch>/` to produce the final `kernel-<protocol>.elf`.

## Registry.cmake

Tracks all registered targets, verifies the isolation rules and writes a Mermaid dependency graph to `docs/src/generated/deps-<arch>.md`.

## Targets.cmake

Defines all the (phony) targers used for creating iso, running and debuggung the kernel.
