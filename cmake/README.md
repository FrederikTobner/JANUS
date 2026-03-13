# cmake - CMake Build Helpers

CMake modules and helper functions for JANUS build system.

## Purpose

Provides toolchain files for cross-compilation, platform detection, and build helper
functions to simplify module CMakeLists.txt files.

## Contents

### toolchains/

Toolchain files for each compiler/architecture combination. These are processed
*before* `project()` and set the compiler, linker, and binutils.

- `aarch64-gcc.cmake` — GCC cross-compiler for aarch64
- `aarch64-clang.cmake` — Clang cross-compiler for aarch64
- `x86_64-gcc.cmake` — GCC for x86_64 (native)
- `x86_64-clang.cmake` — Clang for x86_64

Usage:

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-clang.cmake
# Or use a preset: cmake --preset aarch64-clang
```

### JanusPlatform.cmake

- Host platform detection
- Compiler ID detection and GCC/architecture validation
- Includes arch-specific flags from `arch/<arch>/JanusPlatform.cmake`
- Common compiler flags (freestanding, no-stdlib, warnings, etc.)
- Build type configuration (Debug, Release, MinSizeRel)

### arch/\<arch\>/JanusPlatform.cmake

Architecture-specific compiler flags and boot protocol configuration. No compiler
or linker selection — that is handled by toolchain files.

### JanusSubsys.cmake

- `janus_add_subsys()` - Add a kernel subsystem with automatic arch detection

**Key Features:**

- **Auto-detects arch/ folders** - no manual `HAS_ARCH` flag needed
- **Enforces subsystem isolation** - FATAL_ERROR if subsystem depends on subsystem (except kmain)
- **Transitive includes** - PUBLIC include paths propagate automatically
- **Globs arch sources** - .c files from `arch/<ARCH>/` are included automatically

**Usage:**

```cmake
# Simple - CMake figures out the rest
janus_add_subsys(drivers
    SOURCES tty.c serial.c
    DEPENDENCIES lib  # Only lib allowed, not other subsystems!
)
```

### JanusKernel.cmake

- `janus_link_kernel()` - Link kernel.elf from subsystems

## Subsystem Directory Structure

When a subsystem has architecture-specific code:

```
subsys/foo/
├── include/foo/              # Tier 1: Public API
├── arch/
│   ├── include/arch/foo/     # Tier 2: Contract headers
│   └── x86_64/
│       ├── include/arch/impl/foo/  # Tier 3: Implementation
│       └── *.c               # Arch source files
└── *.c                       # Generic source files
```

The `janus_add_subsys()` function automatically sets up include paths for all three tiers.
