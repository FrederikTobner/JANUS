# JANUS Kernel Libraries

This directory contains utility libraries used by the JANUS kernel. Each library is a separate module with clear responsibilities and no dependencies besides the kernel main headers (kernel/include/janus). E.g. String an memory operations, data structures. compression, decompression, checksum calculation, hashing, formatted I/O and math or general algorithms. Additionally parsers.
Only a stub for now.

## Library Modules

### memory

**Low-level memory manipulation**

Provides essential memory operations without libc:

- memcpy - Copy memory regions
- memmove - Copy with overlap support
- memset - Fill memory with byte value
- memcmp - Compare memory regions
- memzero - Zero-fill memory

### buffer

**Character buffer operations**

character buffer operations:

- string_escape_mem, string_get_size

### fio

**Formatted input/output**

Formatted I/O for kernel logging and debugging:

- Printf-style formatting
- Debug output
- Hex dumps formatting

Printing could be done via prooviding callback functions so that we can keep the kernel and formatting seperated

## Naming Convention

Libraries are in `lib/` directory with simple, descriptive names:

- **memory** - Memory operations (not "mem" or "string")
- **buffer** - Character buffers (we prefer "character buffer" over "string")
- **fio** - Formatted I/O (explicit about being formatted)

The `lib/` directory itself makes it clear these are libraries. No need for additional prefixes.

## Build System

Each library builds as a static library (.a file) with CMake:

```cmake
# lib/foo/CMakeLists.txt
janus_add_library(foo STATIC foo.c)
```

Libraries are linked into the final kernel binary.

## Testing

Tests live with the code they test:

```
lib/buffer/
├── char_buffer.c
├── char_buffer_test.c       # Tests (if BUILD_TESTS=ON)
└── CMakeLists.txt
```
