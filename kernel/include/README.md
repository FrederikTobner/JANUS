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
└── uapi/                # User-space API headers 
```

## Usage

```c
// For kernel code needing global types
#include <tinyos/types.h>

phys_addr_t phys_addr = 0x100000;
virt_addr_t virt_addr = 0xFFFFFFFF80000000;

// For version information
#include <tinyos/version.h>

const char * version = tinyos_version_string();
```

