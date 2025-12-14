# include - Global Headers

Global, cross-cutting header files used throughout TinyOS.

## Purpose

Provides kernel-wide type definitions, configuration, and version information accessible to all modules.

## Structure

```
include/
├── tinyos/              # Main OS interface
│   ├── types.h          # Global kernel types (phys_addr_t, virt_addr_t, pid_t, etc.)
│   ├── config.h.in      # Build configuration template (processed by CMake)
│   ├── config.h         # Generated build configuration (in build/include/)
│   └── version.h        # Version information and queries
└── uapi/                # User-space API headers (future)
```

## Contents

### tinyos/types.h
Kernel-specific type definitions:
- Memory addresses: `phys_addr_t`, `virt_addr_t`
- Page frame number: `pfn_t`
- Process/thread IDs: `pid_t`, `tid_t`
- Device number: `dev_t`
- Error type: `error_t`

### tinyos/config.h
Build-time configuration (generated from config.h.in):
- Project name and version
- Build date and time
- Compiler information
- Feature flags (BUILD_TESTS, ENABLE_DEBUG_OUTPUT, etc.)

### tinyos/version.h
Version query functions:
- `tinyos_version_string()`
- `tinyos_full_version_string()`
- `tinyos_version_at_least(major, minor, patch)`

## Usage

```c
// For kernel code needing global types
#include <tinyos/types.h>

phys_addr_t phys_addr = 0x100000;
virt_addr_t virt_addr = 0xFFFFFFFF80000000;

// For version information
#include <tinyos/version.h>

const char* version = tinyos_version_string();
```

## Dependencies

- `tinyos/types.h` depends on `lib/types.h`
- `tinyos/version.h` depends on `tinyos/config.h`

## Status

✅ **Active** - Core headers implemented and in use.
