# TinyOS Include Hierarchy and Header Organization

## Include Philosophy

> "Every program has at least one bug and can be shortened by at least one instruction—from which, by induction, it can be deduced that every program can be reduced to one instruction which doesn't work."
> — *Unknown*

**If your header doesn't compile on its own, it's broken. If it exposes internal details, it's wrong.**

Headers must be:
- **Self-contained** - Include all dependencies. Don't make users guess what to include.
- **Minimal** - Only expose what's necessary. Internal shit stays internal.
- **Well-organized** - Clear public vs private separation. No dumping everything in one header.
- **Portable** - Work across architectures when possible. Architecture-specific stuff goes in arch/.

## Include Directory Structure

```
TinyOS/
├── include/                   # Global, cross-cutting headers
│   ├── tinyos/               # Main OS interface
│   │   ├── types.h           # Global type definitions
│   │   ├── config.h          # Build configuration
│   │   ├── version.h         # Version information
│   │   └── compiler.h        # Compiler-specific defines
│   └── uapi/                 # User-space API (future)
│
├── kernel/include/           # Kernel module headers
│   └── kernel/
│       ├── kernel.h
│       ├── panic.h
│       └── init.h
│
├── arch/include/             # Architecture headers
│   └── arch/
│       ├── types.h           # Arch-agnostic types
│       ├── cpu.h             # CPU interface
│       ├── io.h              # I/O operations
│       ├── serial.h          # Serial port
│       └── vga.h             # VGA text mode
│
├── lib/libkbuffer/include/   # Library headers
│   └── lib/
│       └── buffer.h
│
├── lib/libkio/include/
│   └── lib/
│       └── io.h
│
└── mm/include/               # Memory management (future)
    └── mm/
        ├── page_frame.h
        ├── vmm.h
        └── kmalloc.h
```

## Include Path Rules

### 1. Global Headers
Use angle brackets for global headers:
```c
#include <tinyos/types.h>
#include <tinyos/config.h>
```

### 2. Module Headers
Use angle brackets with module path:
```c
#include <lib/buffer.h>
#include <lib/io.h>
#include <arch/cpu.h>
#include <mm/page_frame.h>
```

### 3. Local Module Headers
Use quotes for headers within the same module:
```c
// In kernel/main.c
#include "kernel/kernel.h"    // Public header (in kernel/include/)
#include "internal.h"          // Private header (in kernel/)
```

## Include Order

### Standard Include Order
```c
// 1. Corresponding header (for .c files)
#include "buffer.h"

// 2. Global TinyOS headers
#include <tinyos/types.h>
#include <tinyos/config.h>

// 3. Library headers
#include <lib/io.h>

// 4. Architecture headers  
#include <arch/cpu.h>

// 5. Other module headers
#include <mm/page_frame.h>

// 6. Private/local headers
#include "internal.h"

// Blank line between groups
```

### Example
```c
// lib/libkbuffer/buffer.c
#include "lib/buffer.h"        // Corresponding header

#include <tinyos/types.h>      // Global headers
#include <tinyos/compiler.h>

#include <lib/io.h>            // Other library headers

#include <arch/cpu.h>          // Architecture headers

#include "buffer_internal.h"   // Private headers
```

## Header File Structure

### Public Header Template
```c
// lib/libkbuffer/include/lib/buffer.h
#ifndef TINYOS_LIB_BUFFER_H
#define TINYOS_LIB_BUFFER_H

// Include dependencies
#include <tinyos/types.h>

// Forward declarations (if needed)
struct char_buffer_view;

// Type definitions
typedef struct char_buffer {
    char*  data;
    size_t length;
    size_t capacity;
} char_buffer_t;

typedef struct char_buffer_view {
    char const * data;
    size_t       length;
} char_buffer_view_t;

// Public API declarations

/**
 * Create a new character buffer
 * @param capacity Initial capacity in bytes
 * @return Pointer to buffer, or NULL on failure
 */
char_buffer_t* kbuf_create(size_t capacity);

/**
 * Destroy a character buffer
 * @param buffer Buffer to destroy
 */
void kbuf_destroy(char_buffer_t* buffer);

// More function declarations...

#endif  // TINYOS_LIB_BUFFER_H
```

### Private Header Template
```c
// lib/libkbuffer/buffer_internal.h
#ifndef BUFFER_INTERNAL_H
#define BUFFER_INTERNAL_H

#include "lib/buffer.h"  // Public interface

// Internal constants
#define BUFFER_DEFAULT_CAPACITY 256
#define BUFFER_GROWTH_FACTOR    2

// Internal helper functions
static inline bool buffer_needs_realloc(char_buffer_t const * buf, size_t additional) {
    return (buf->length + additional) > buf->capacity;
}

size_t buffer_compute_new_capacity(size_t current, size_t required);
void buffer_internal_realloc(char_buffer_t* buf, size_t new_capacity);

#endif  // BUFFER_INTERNAL_H
```

## Header Guards

### Naming Convention
```c
// Format: TINYOS_<PATH>_<FILENAME>_H
// Path components separated by underscores, uppercase

// include/tinyos/types.h
#ifndef TINYOS_TYPES_H
#define TINYOS_TYPES_H
// ...
#endif

// kernel/include/kernel/panic.h
#ifndef TINYOS_KERNEL_PANIC_H
#define TINYOS_KERNEL_PANIC_H
// ...
#endif

// lib/libkbuffer/include/lib/buffer.h
#ifndef TINYOS_LIB_BUFFER_H
#define TINYOS_LIB_BUFFER_H
// ...
#endif

// arch/include/arch/cpu.h
#ifndef TINYOS_ARCH_CPU_H
#define TINYOS_ARCH_CPU_H
// ...
#endif
```

### Private Headers
```c
// Local headers can use simpler guards
// lib/libkbuffer/buffer_internal.h
#ifndef BUFFER_INTERNAL_H
#define BUFFER_INTERNAL_H
// ...
#endif
```

## Forward Declarations

Use forward declarations to minimize header dependencies:

```c
// Instead of including the full header
// BAD:
#include <mm/page_frame.h>  // Just to get page_frame_t*

typedef struct foo {
    page_frame_t* frame;  // Only using pointer
} foo_t;

// GOOD: Forward declare
struct page_frame;  // Forward declaration

typedef struct foo {
    struct page_frame* frame;  // Pointer works without full definition
} foo_t;
```

## Architecture-Specific Headers

### Conditional Inclusion
```c
// arch/include/arch/cpu.h
#ifndef TINYOS_ARCH_CPU_H
#define TINYOS_ARCH_CPU_H

#include <tinyos/types.h>

// Include architecture-specific implementation
#ifdef __x86_64__
#include "arch/x86_64/cpu.h"
#elif defined(__aarch64__)
#include "arch/aarch64/cpu.h"
#else
#error "Unsupported architecture"
#endif

#endif  // TINYOS_ARCH_CPU_H
```

### Architecture-Specific Implementation Header
```c
// arch/x86_64/include/arch/x86_64/cpu.h
#ifndef TINYOS_ARCH_X86_64_CPU_H
#define TINYOS_ARCH_X86_64_CPU_H

#include <tinyos/types.h>

// x86-64 specific definitions
typedef struct cpu_regs {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
} cpu_regs_t;

// x86-64 specific functions
static inline void arch_enable_interrupts(void) {
    __asm__ volatile ("sti");
}

#endif  // TINYOS_ARCH_X86_64_CPU_H
```

## CMake Include Path Configuration

### Module Include Directories
```cmake
# lib/libkbuffer/CMakeLists.txt
add_kernel_library(kbuffer
    SOURCES buffer.c
)

# Export public include directory
target_include_directories(kbuffer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>  # Global headers
)
```

### Global Include Setup
```cmake
# Root CMakeLists.txt
include_directories(
    ${CMAKE_SOURCE_DIR}/include          # Global headers
)

# Each module adds its own public headers
add_subdirectory(kernel)                 # Adds kernel/include/
add_subdirectory(arch)                   # Adds arch/include/
add_subdirectory(lib/libkbuffer)         # Adds lib/libkbuffer/include/
```

## Header Documentation

### Header File Comments
```c
/**
 * @file buffer.h
 * @brief Character buffer manipulation and views
 * 
 * This module provides safe character buffer operations including
 * creation, destruction, manipulation, and view generation.
 * 
 * @author TinyOS Team
 * @date 2025
 */
#ifndef TINYOS_LIB_BUFFER_H
#define TINYOS_LIB_BUFFER_H
```

### Type Documentation
```c
/**
 * Character buffer structure
 * 
 * Represents a growable character buffer with automatic memory management.
 * Buffers maintain both length (current size) and capacity (allocated size).
 */
typedef struct char_buffer {
    char*  data;      ///< Pointer to buffer data
    size_t length;    ///< Current buffer length in bytes
    size_t capacity;  ///< Total allocated capacity in bytes
} char_buffer_t;
```

## Include Best Practices

### 1. Minimize Dependencies
Only include what you need:
```c
// BAD: Including everything
#include <lib/buffer.h>
#include <lib/io.h>
#include <arch/cpu.h>
#include <mm/page_frame.h>

// GOOD: Only what's needed
#include <lib/buffer.h>  // Used in this file
```

### 2. Use Forward Declarations
```c
// BAD: Full header for pointer-only usage
#include "complex_structure.h"

// GOOD: Forward declaration
struct complex_structure;
void process(struct complex_structure* cs);
```

### 3. Keep Headers Self-Contained
Each header should include its dependencies:
```c
// buffer.h
#ifndef TINYOS_LIB_BUFFER_H
#define TINYOS_LIB_BUFFER_H

#include <tinyos/types.h>  // Needed for size_t

// Now buffer.h can be included anywhere without prerequisites
```

### 4. Avoid Circular Dependencies
```c
// BAD: A includes B, B includes A
// a.h
#include "b.h"

// b.h  
#include "a.h"

// GOOD: Use forward declarations
// a.h
struct b;  // Forward declaration

// b.h
struct a;  // Forward declaration
```