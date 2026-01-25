# JANUS Coding Style Guide

## Overview

JANUS uses `clang-format` to enforce consistent style. If you're coming from C++, Rust, or Zig, most of this will be familiar. We only document the non-obvious parts here - things that might surprise you or are specific to kernel development.

## Multi-Dimensional Memory Access

### Pointer Arithmetic for Multi-Dimensional Data

JANUS prefers **explicit pointer arithmetic** over array indexing for multi-dimensional data structures to make memory layout and performance characteristics visible.

```c
// Preferred: Explicit pointer arithmetic
u32 * pixel_ptr = framebuffer + (y * pitch) + x;
*pixel_ptr = color;

// Also acceptable: shows the calculation clearly
u8 * pixel_base = framebuffer + (y * pitch) + (x * bytes_per_pixel);
u8 r = *(pixel_base + 0);
u8 g = *(pixel_base + 1);
u8 b = *(pixel_base + 2);
u8 a = *(pixel_base + 3);

// Avoid: Hidden memory layout
u32 color = framebuffer[y][x];  // Where's the pitch? Is this cache-friendly?
```

```c
// Preferred: Cache behavior visible
void matrix_multiply(float * result, float const * a, float const * b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            float * result_elem = result + (i * n) + j;
            *result_elem = 0.0f;
            
            for (size_t k = 0; k < n; k++) {
                // Shows: a[i,k] is sequential, b[k,j] jumps by n
                *result_elem += *(a + i * n + k) * *(b + k * n + j);
            }
        }
    }
}

// Avoid: Hidden memory access patterns
result[i][j] += a[i][k] * b[k][j];  // How far apart are these elements?
```

#### One-Dimensional Arrays: Array Syntax OK

For simple 1D arrays, traditional indexing is fine:

```c
// OK: Simple 1D array
char buffer[256];
buffer[i] = 'x';

// Also OK for simple sequential access
for (size_t i = 0; i < count; i++) {
    array[i] = initial_value;
}
```

**Key Principle**: If the memory layout and access pattern matter for performance or understanding, make them explicit.

## Structure Encapsulation Philosophy

### Public Structures

**JANUS uses public structure definitions rather than opaque handles.**

Opaque handles (`typedef struct foo * foo_handle_t`) hide structure definitions from callers. This pattern is common in userspace libraries for API stability, but creates significant problems in kernel development:

**Performance Concerns:**

- Every field access requires pointer indirection
- Forces heap allocation instead of enabling stack allocation
- Reduces cache locality by requiring separate allocations
- Adds function call overhead for simple field access

**Debugging nightmare:**

```bash
# Public structure - see everything
(lldb) print buffer
(kbuffer_t) {
  data = 0x7fff "Hello, World!"
  length = 13
  capacity = 256
}

# Opaque handle - see nothing
(lldb) print handle
(void *) 0x7fff8000  # Congratulations, you have an address
```

**Memory waste:**

```c
// Public structure - stack allocation, zero malloc overhead
kbuffer_t buffer;
kbuf_init(&buffer, 256);

// Opaque handle - forces heap allocation you don't need
handle_t * handle = handle_create(256);  // Pointless malloc
```

**Cache murder:**

```c
// Public structure - embedding means one allocation
typedef struct parser {
    kbuffer_t input;   // Embedded, contiguous memory
    kbuffer_t output;  // Better cache locality
    size_t pos;
} parser_t;

// Opaque handle - pointer chasing across memory
typedef struct parser {
    handle_t * input;   // Pointer to heap allocation
    handle_t * output;  // Another pointer to different heap allocation
    size_t pos;
} parser_t;  // Likely worse cache behavior
```

**Encapsulation in kernel code:**

Kernel developers need to understand data structure internals. Hiding implementation details creates barriers to debugging and optimization without providing meaningful benefits. Public structures enable:

- Direct field access for performance-critical code
- Stack allocation to avoid memory management overhead
- Structure embedding for better cache behavior
- Full visibility during debugging

### Default: Public Structures

**Core data structures use public definitions:**

```c
// include/buffer/buffer.h - Public structure definition
typedef struct buffer {
    uint8_t * data;
    size_t length;
    size_t capacity;
} buffer_t;

// Inline helpers for convenience
static inline size_t buf_length(buffer_t const * buf) {
    return buf->length;
}

// Complex operations as functions
void buf_init(buffer_t * buf, size_t initial_capacity);
int buf_append(buffer_t * buf, uint8_t const * data, size_t len);
void buf_destroy(buffer_t * buf);
```

### Opaque Handles: The Rare Exception

Use opaque handles ONLY when you have **actual** abstraction needs:

```c
// Hardware varies wildly - VGA text vs VESA graphics vs EFI framebuffer
typedef struct framebuffer * framebuffer_handle_t;

// Plugin interface where external code must work with future changes
typedef struct fs_driver * fs_driver_handle_t;
```

**DO NOT use opaque handles for:**

- Core data structures (buffers, lists, hashtables)
- Anything performance-critical
- Anything that needs stack allocation
- Anything where you want developers to understand the layout

If you find yourself typing `typedef struct foo * foo_handle_t` for a buffer or list or tree, **stop**. You're about to write bad code.

### Document Your Structures

Since structures are public, document them properly:

```c
/**
 * Page table entry (x86-64)
 * 
 * Bit layout:
 *   0-11:   Flags
 *   12-51:  Physical page frame number  
 *   52-62:  Available for OS
 *   63:     Execute disable
 * 
 * Invariants:
 *   - Physical address is 4KB aligned
 *   - If !PRESENT, other flags are undefined
 * 
 * Direct bit manipulation allowed.
 */
typedef struct page_table_entry {
    uint64_t value;
} pte_t;

static inline bool pte_is_present(pte_t const * pte) {
    return (pte->value & PTE_PRESENT) != 0;
}

// Direct manipulation when you know what you're doing
pte->value = phys_addr | PTE_PRESENT | PTE_WRITABLE;
```
