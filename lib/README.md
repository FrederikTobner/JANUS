# TinyOS Kernel Libraries

This directory contains utility libraries used by the TinyOS kernel. Each library is a separate module with clear responsibilities and dependencies.
Only a stub for now.

## Library Modules

### types

**Fixed-width types and compiler attributes**

Header-only library providing:

- Fixed-width integer types (uint8_t, int32_t, etc.)
- Size types (size_t, ssize_t, ptrdiff_t)
- Boolean type (bool, true, false)
- NULL definition
- Compiler attributes (__packed,__aligned, __noreturn, etc.)
- Common macros (likely, unlikely, ARRAY_SIZE, min, max)

**Dependencies:** None (header-only)

### memory

**Low-level memory manipulation**

Provides essential memory operations without libc:

- memcpy - Copy memory regions
- memmove - Copy with overlap support
- memset - Fill memory with byte value
- memcmp - Compare memory regions
- memzero - Zero-fill memory

**Dependencies:** types

### buffer

**Character buffer operations**

Dynamic character buffer with safe operations:

- Buffer creation and destruction
- Append, insert, delete operations
- Buffer views and slicing
- Memory-safe operations

**Dependencies:** memory, types

### fio

**Formatted input/output**

Formatted I/O for kernel logging and debugging:

- Printf-style formatting
- Kernel logging (printk)
- Debug output
- Hex dumps

**Dependencies:** buffer, types

## Dependency Graph

```
fio
├── depends on: buffer
│
buffer
├── depends on: memory
│
memory
└── depends on: types (header-only)
```

## Naming Convention

Libraries are in `lib/` directory with simple, descriptive names:

- **types** - Basic types and compiler attributes
- **memory** - Memory operations (not "mem" or "string")
- **buffer** - Character buffers (we prefer "character buffer" over "string")
- **fio** - Formatted I/O (explicit about being formatted)

The `lib/` directory itself makes it clear these are libraries. No need for additional prefixes.

## Build System

Each library builds as a static library (.a file) with CMake:

```cmake
# lib/memory/CMakeLists.txt
add_library(memory STATIC memory.c)
target_link_libraries(memory PUBLIC types)
```

Libraries are linked into the final kernel binary.

## Testing

Tests live with the code they test:

```
lib/buffer/
├── buffer.c
├── buffer_test.c       # Tests (if BUILD_TESTS=ON)
└── CMakeLists.txt
```

See [BMUnit-Testing.md](../Documentation/Implementation/BMUnit-Testing.md) for testing practices.
