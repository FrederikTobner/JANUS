# Coding Style

JANUS enforces a consistent coding style via `clang-format`. This page documents the conventions that go beyond what the formatter handles — architectural patterns, memory access style, and encapsulation policy.

## Multi-Dimensional Memory Access

JANUS prefers explicit pointer arithmetic over array indexing for multi-dimensional data structures. The intent is to make memory layout and access patterns visible at the point of use, so that cache behaviour and stride are immediately apparent to anyone reading the code.

```c
// Preferred: layout and stride are explicit
u32 *pixel_ptr = framebuffer + (y * pitch) + x;
*pixel_ptr = color;
```

```c
// Preferred: cache behaviour visible
void matrix_multiply(float *result, float const *a, float const *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            float *result_elem = result + (i * n) + j;
            *result_elem = 0.0f;

            for (size_t k = 0; k < n; k++) {
                // a[i,k] is sequential; b[k,j] jumps by n
                *result_elem += *(a + i * n + k) * *(b + k * n + j);
            }
        }
    }
}
```

```c
// Avoid: the pitch is hidden inside the subscript operator
u32 color = framebuffer[y][x];
```

For simple one-dimensional arrays, standard subscript notation is perfectly acceptable:

```c
char buffer[256];
buffer[i] = 'x';
```

The dividing line is straightforward: if the memory layout matters for correctness or performance, make it explicit. If it does not, use whichever notation is clearest.

## Structure Encapsulation

Kernel data structures are defined publicly by default. The rationale is documented in [Philosophy](../principles/philosophy.md), but the practical rules are:

- Define structures in the public header so that consumers can allocate them on the stack, embed them in other structures, and inspect them in a debugger.
- Provide inline accessors when a convenient shorthand is useful, but do not hide fields behind getter/setter functions purely for encapsulation.
- Reserve opaque handles (`typedef struct foo *foo_handle_t`) for the rare case where a genuine abstraction boundary exists — typically hardware backends where the underlying representation varies at runtime.

Opaque handles carry concrete costs in kernel code:

```bash
# Public structure — see everything
(lldb) print buffer
(kbuffer_t) {
  data = 0x7fff "Hello, World!"
  length = 13
  capacity = 256
}

# Opaque handle — see nothing
(lldb) print handle
(void *) 0x7fff8000
```

```c
// Public structure — stack allocation, zero malloc overhead
kbuffer_t buffer;
kbuf_init(&buffer, 256);

// Opaque handle — forces heap allocation you don't need
handle_t *handle = handle_create(256);
```

```c
// Public structure — embedding means one allocation, contiguous memory
typedef struct parser {
    kbuffer_t input;
    kbuffer_t output;
    size_t pos;
} parser_t;

// Opaque handle — pointer chasing across separate heap allocations
typedef struct parser {
    handle_t *input;
    handle_t *output;
    size_t pos;
} parser_t;
```

**Do not** use opaque handles for core data structures (buffers, lists, hashtables), anything performance-critical, anything that benefits from stack allocation, or anything where developers need to understand the layout. Reserve them for the rare case where the underlying representation genuinely varies at runtime:

```c
typedef struct framebuffer *framebuffer_handle_t;
typedef struct fs_driver *fs_driver_handle_t;
```

## Documentation

Since structures are public, document them properly. Every structure that appears in a public header should carry a Doxygen comment explaining its fields, invariants, and layout constraints:

```c
/**
 * Page table entry (x86-64).
 *
 * Bits 0–11: flags. Bits 12–51: physical page frame number.
 * Bits 52–62: available for OS use. Bit 63: execute disable.
 *
 * Invariants:
 *   - The physical address is 4 KiB aligned.
 *   - If PRESENT is clear, all other flags are undefined.
 */
typedef struct page_table_entry {
    uint64_t value;
} pte_t;
```

## East-Const

JANUS uses east-const placement throughout:

```c
char const *message;          // pointer to const char
boot_context_t const *ctx;    // pointer to const boot_context_t
```

This is enforced by convention, not by the formatter. The rationale is that east-const reads consistently left-to-right and avoids the ambiguity of `const char *` versus `char *const`.
