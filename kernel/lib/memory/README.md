# memory - Memory Manipulation Operations

Low-level memory manipulation functions for kernel use.

## Purpose

Provides essential memory operations (copy, move, set, compare) without depending on standard C library.

## Contents

### memory.h / memory.c
- `memcpy()` - Copy memory (non-overlapping)
- `memmove()` - Copy memory (overlapping-safe)
- `memset()` - Fill memory with byte value
- `memcmp()` - Compare memory regions
- `memzero()` - Zero-fill memory

## Key Features

- **Freestanding**: No libc dependency
- **Optimized**: Efficient implementations for x86_64
- **Safe**: Handles edge cases and null pointers

## Dependencies

- `types` - Basic type definitions

## Usage Example

```c
#include <lib/memory.h>

uint8_t buffer[256];
memzero(buffer, sizeof(buffer));

uint8_t src[64], dst[64];
memcpy(dst, src, 64);

if (memcmp(dst, src, 64) == 0) {
    // Buffers are equal
}
```

## Status

✅ **Active** - Core implementation complete, additional optimizations planned.
