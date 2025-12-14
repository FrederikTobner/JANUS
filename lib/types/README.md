# types - Basic Type Definitions

Freestanding replacement for standard C library type headers.

## Purpose

Provides fundamental type definitions required for kernel development without depending on the standard C library.

## Contents

### types.h
- Fixed-width integer types (`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`)
- Pointer-sized integers (`intptr_t`, `uintptr_t`)
- Size types (`size_t`, `ssize_t`, `ptrdiff_t`)
- Boolean type (`bool`, `true`, `false`)
- NULL pointer definition
- Integer limits macros
- `offsetof` macro

### compiler.h
- Compiler attributes (`__packed`, `__aligned`, `__noreturn`, `__unused`, etc.)
- Branch prediction hints (`likely()`, `unlikely()`)
- Memory barriers (`barrier()`)
- Utility macros (`ARRAY_SIZE()`, `container_of()`, `min()`, `max()`)
- Static assertions
- Compiler detection

## Replaces

- `<stdint.h>` - Fixed-width integer types
- `<stddef.h>` - Basic type definitions
- `<stdbool.h>` - Boolean type

## Dependencies

None - this is a foundational header-only library.

## Usage

```c
#include <lib/types.h>
#include <lib/compiler.h>

uint32_t value = 42;
bool flag = true;
size_t len = ARRAY_SIZE(array);
```

See [Documentation/Code-Organization/Type-System.md](../../Documentation/Code-Organization/Type-System.md) for detailed usage patterns.
