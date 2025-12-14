# cmake - CMake Build Helpers

CMake modules and helper functions for TinyOS build system.

## Purpose

Provides platform detection, cross-compilation setup, and build helper functions to simplify module CMakeLists.txt files.

## Contents

### TinyOSPlatform.cmake
- Platform and compiler detection
- Cross-compilation toolchain setup for x86_64-elf
- Compiler flags configuration (freestanding, no-stdlib, etc.)
- Build type configuration (Debug, Release)
- Build information printing

### TinyOSHelpers.cmake
- `tinyos_add_library()` - Add a library module with automatic configuration
- `tinyos_add_module()` - Add a kernel module with dependencies
- `tinyos_add_test()` - Add BMUnit test suite
- `tinyos_add_tests()` - Batch test registration
- `tinyos_print_config()` - Display build configuration

## Key Features

- **Automatic Configuration**: Libraries and modules get correct compiler flags automatically
- **Dependency Management**: Declarative dependency specification
- **Cross-Platform**: Works on Linux, macOS, and other Unix-like systems
- **Consistent Builds**: All modules use same compiler settings

## Usage Example

```cmake
# In a module CMakeLists.txt
tinyos_add_library(buffer
    SOURCES
        buffer.c
    DEPENDENCIES
        types
        memory
)

tinyos_add_test(buffer_test
    SOURCES buffer_test.c
    DEPENDS buffer
)
```

## Status

✅ **Active** - Core build system complete and stable.
