# TinyOS Coding Style Guide

## Overview

TinyOS uses `clang-format` to enforce consistent style. If you're coming from C++, Rust, or Zig, most of this will be familiar. We only document the non-obvious parts here - things that might surprise you or are specific to kernel development.

## Basic Formatting (Quick Reference)

**Don't waste time reading this if you know C:**
- 4 spaces, no tabs
- 120 column limit
- Linux kernel brace style
- Pointer alignment middle: `uint8_t * ptr`
- Qualifiers right: `uint32_t const value`
- Space after casts: `(uint32_t) value`
- Always use braces, even for single statements

Run `clang-format -i <file>` and move on.

## Multi-Dimensional Memory Access

### Pointer Arithmetic for Multi-Dimensional Data

TinyOS prefers **explicit pointer arithmetic** over array indexing for multi-dimensional data structures to make memory layout and performance characteristics visible.

#### Framebuffer and Pixel Data

```c
// Preferred: Explicit pointer arithmetic
uint32_t * pixel_ptr = framebuffer + (y * pitch) + x;
*pixel_ptr = color;

// Also acceptable: shows the calculation clearly
uint8_t * pixel_base = framebuffer + (y * pitch) + (x * bytes_per_pixel);
uint8_t r = *(pixel_base + 0);
uint8_t g = *(pixel_base + 1);
uint8_t b = *(pixel_base + 2);
uint8_t a = *(pixel_base + 3);

// Avoid: Hidden memory layout
uint32_t color = framebuffer[y][x];  // Where's the pitch? Is this cache-friendly?
```

#### Matrix Operations

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

#### Page Tables and Nested Structures

```c
// Preferred: Shows multi-level pointer traversal
uint64_t * pml4 = (uint64_t *) read_cr3();
uint64_t pml4_entry = *(pml4 + pml4_index);

if (pml4_entry & PAGE_PRESENT) {
    uint64_t * pdpt = (uint64_t *) (pml4_entry & PAGE_ADDR_MASK);
    uint64_t pdpt_entry = *(pdpt + pdpt_index);
    // ...
}

// Shows exactly what memory is being accessed at each level
```

#### Helper Macros for Common Patterns

When pointer arithmetic becomes repetitive, use macros:

```c
// Define access pattern once
#define PIXEL_AT(fb, pitch, x, y) ((fb) + ((y) * (pitch)) + (x))

// Use consistently
uint32_t * pixel = PIXEL_AT(framebuffer, pitch, x, y);
*pixel = color;

// Or for 2D matrices
#define MATRIX_ELEM(matrix, cols, row, col) ((matrix) + ((row) * (cols)) + (col))

float * element = MATRIX_ELEM(matrix, num_cols, i, j);
*element = value;
```

#### When to Comment the Layout

```c
/*
 * Framebuffer layout:
 * - Width: 1024 pixels
 * - Height: 768 pixels  
 * - Pitch: 1024 pixels (may differ if aligned)
 * - Format: 32-bit RGBA
 * - Memory: width * height * 4 bytes
 * - Row N starts at: base + (N * pitch * 4)
 */
uint32_t * pixel = framebuffer + (y * pitch) + x;
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

### Don't Hide Your Damn Structures

TinyOS follows the Linux kernel approach: **expose structure definitions**. No opaque handle bullshit.

**Why? Because Linus is right:**

> "In kernel space, we don't hide data structures. If you can't trust your kernel developers to not mess with internal fields, you have the wrong developers." - Linus Torvalds

Opaque handles are a disease imported from userspace OOP garbage. They:
- **Kill performance** with pointless indirection
- **Murder the cache** by forcing heap allocation and pointer chasing  
- **Fuck debugging** - try inspecting `void *` in a debugger
- **Waste memory** on unnecessary allocations
- Are a **solution looking for a problem** in kernel code

If you need to hide implementation details from your own kernel developers, you don't have a code problem - you have a people problem.

### Default: Public Structures

**All core data structures are public. Period.**

```c
// include/kbuffer/buffer.h - Structure is PUBLIC, deal with it
typedef struct kbuffer {
    uint8_t * data;
    size_t length;
    size_t capacity;
} kbuffer_t;

// Inline helpers for convenience (not for "encapsulation")
static inline size_t kbuf_length(kbuffer_t const * buf) {
    return buf->length;
}

// Complex operations as functions
void kbuf_init(kbuffer_t * buf, size_t initial_capacity);
int kbuf_append(kbuffer_t * buf, uint8_t const * data, size_t len);
void kbuf_destroy(kbuffer_t * buf);
```

### Why This is Better

**Performance:**
```c
kbuffer_t buffer;  // Stack allocation - zero malloc overhead
kbuf_init(&buffer, 256);

if (buffer.length > 0) {  // Direct access - zero function call overhead
    process(buffer.data);
}
```

**Debugging:**
```bash
(lldb) print buffer
(kbuffer_t) $0 = {
  data = 0x00007ffff7fb0000 "actual data you can see"
  length = 23
  capacity = 256
}

# vs opaque handle stupidity:
(lldb) print handle
(void *) $0 = 0x00007ffff7fb0000  # Great, an address. Very helpful.
```

**Memory layout:**
```c
typedef struct parser {
    kbuffer_t input;   // Embedded - one allocation for entire struct
    kbuffer_t output;  // Not a pointer - better cache locality
    size_t pos;
} parser_t;
```

### When Direct Access is Expected

Direct field access is **normal and encouraged**:

```c
// Hot loop? Access fields directly.
for (size_t i = 0; i < buffer.length; i++) {
    process(buffer.data[i]);
}

// Need to check something? Just check it.
if (buffer.capacity < required) {
    kbuf_resize(&buffer, required);
}

// Setting up a structure? Set the damn fields.
packet.payload.length = new_size;
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

### Naming

**Public structures:**
```c
typedef struct kbuffer kbuffer_t;
void kbuf_init(kbuffer_t * buf, size_t capacity);
void kbuf_destroy(kbuffer_t * buf);
```

**Opaque handles (rare):**
```c
typedef struct device * device_handle_t;
device_handle_t device_create(device_type_t type);
void device_destroy(device_handle_t handle);
```

## Editor Integration
Configure your editor to run clang-format on save:

**VS Code** (`.vscode/settings.json`):
```json
{
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_path": "clang-format",
    "C_Cpp.clang_format_style": "file"
}
```

**Vim/Neovim**:
```vim
autocmd BufWritePre *.c,*.h :silent! !clang-format -i %
```