# cmake - CMake Build Helpers

CMake modules and helper functions for JANUS build system.

## Purpose

Provides platform detection, cross-compilation setup, and build helper functions to simplify module CMakeLists.txt files.

## Contents

### JanusPlatform.cmake

- Platform and compiler detection
- Cross-compilation toolchain setup for x86_64-elf
- Compiler flags configuration (freestanding, no-stdlib, etc.)
- Build type configuration (Debug, Release)

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
