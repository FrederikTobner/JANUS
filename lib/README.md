# TinyOS Libraries

Kernel utility libraries providing core functionality.

## libkbuffer
Character buffer manipulation and management.

**Responsibilities:**
- Dynamic buffer allocation and resizing
- Buffer views and slicing
- String-like operations on character buffers
- Memory-safe buffer operations

## libkio
Input/output operations for kernel.

**Responsibilities:**
- Formatted output (printf-style functions)
- Character and buffer I/O
- Debug output and logging
- Stream abstractions

**Dependencies:** libkbuffer

## libkstd
Standard library subset for freestanding environment.

**Responsibilities:**
- Basic type definitions (stdint.h, stddef.h, stdbool.h)
- String manipulation functions (memcpy, memset, strlen, etc.)
- Essential compiler intrinsics
- Platform-independent abstractions

**Note:** This is NOT the full C standard library - only the subset needed for kernel development.
