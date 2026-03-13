# Architecture Layer Structure in JANUS

The architecture (arch) layer in JANUS provides a flexible, modular way to support platform-specific code within any subsystem or library. This document explains the patterns, directory conventions, and best practices for organizing arch code.

## Overview

- **Arch code is co-located**: Each subsystem or library that needs platform-specific code contains its own `arch/` directory.
- **Three-tier model**: Public API (Tier 1), contract/bridge (Tier 2), and implementation (Tier 3).
- **No 1:1 mapping required**: Public headers can include multiple arch contracts; arch implementations can be split across files.

## Directory Structure

```
subsys/example/
├── include/example/            # Tier 1: Public API
│   └── foo.h
├── foo.c                      # Generic implementation (optional)
└── arch/
    ├── include/arch/example/  # Tier 2: Contract headers
    │   ├── foo.h              # Declares arch_* or includes Tier 3
    │   └── bar.h              # ...can include multiple contracts
    └── x86_64/
        ├── include/arch/impl/example/  # Tier 3: Inline arch headers
        │   └── foo.h
        ├── foo.c              # Arch implementation (optional)
        └── bar.c              # Arch implementation (optional)
```

## Key Patterns

- **Header-only (all tiers)**: All logic is inline, no `.c` files needed.
- **Generic header-only, arch `.c`**: Public API is inline, arch provides compiled implementation.
- **Generic `.c`, arch header-only**: Generic code calls inline arch helpers.
- **Both `.c`**: Both generic and arch have compiled code.
- **Multi-file arch**: Arch implementation can be split across multiple files (e.g., `foo_hardware.c`, `foo_init.c`).
- **Multi-contract**: A public header can include several arch contracts (e.g., `foo.h` includes `arch/example/bar.h` and `arch/example/baz.h`).

## Best Practices

- **Namespacing**: Always namespace arch headers by subsystem/library to avoid collisions.
- **Contracts**: Tier 2 headers declare the `arch_*` interface for the current arch, or include Tier 3 inline headers.
- **Flexibility**: You are not required to mirror public API names in the arch layer. Use as many files as needed for clarity and separation of concerns.
- **CMake**: The build system will automatically glob `.c` files from the correct arch directory and set up include paths.

