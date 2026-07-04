# Conventions

## Coding Style

### Multi-Dimensional Memory Access

When accessing a multi-dimensional data structure, prefer explicit pointer arithmetic over chained array subscripts.
This convention exists because the arithmetic makes the memory layout and access stride immediately visible at the
point of use, which is where cache behaviour and correctness are most easily audited:

```c
// Preferred: the pitch and the resulting offset are stated explicitly
u32 * pixel_ptr = framebuffer + (y * pitch) + x;
*pixel_ptr = color;

// Avoid: the pitch is hidden inside the subscript operator and invisible to
// the reader without knowledge of how framebuffer is declared
u32 color = framebuffer[y][x];
```

For plain one-dimensional arrays, standard subscript notation is perfectly acceptable and should be used.

### Structure Encapsulation

Structures are defined publicly in their owning module's header by default, rather than being hidden behind opaque pointer typedefs.
The rationale is covered in depth in [00-overview.md](00-overview.md).
The practical rule is that a struct should only be made opaque when its internal representation genuinely varies at runtime and that variation needs to be hidden from consumers.

### East-Const

`const` is placed to the right of the type specifier throughout the codebase.
This placement reads consistently from left to right and eliminates the ambiguity that arises when `const` precedes a pointer type:

```c
char const * message;          // pointer to const char
boot_context_t const * ctx;    // pointer to const boot_context_t
char * const fixed_ptr;        // const pointer to mutable char
```

### Documentation

Every public structure in a public header must carry a Doxygen `@brief` comment that describes its purpose, documents each field, and states any invariants or layout constraints that a reader cannot infer from the field types alone.

### Static Function Layout

Within every `.c` file, the sections appear in the following order:

1. `static` forward declarations — establishes at a glance which internal helpers
   exist without requiring the reader to scan to the bottom of the file.
2. Public function definitions — the externally visible API, which is what most
   readers are looking for first.
3. `static` function definitions — the implementations of the helpers declared
   at the top.

## Naming

### Type Suffixes

All custom types carry the `_t` suffix. Structure tags match the typedef name without the suffix so that both the tag and the typedef can be used:

```c
typedef struct linked_list linked_list_t;
typedef struct ring_buffer ring_buffer_t;
```

Address types distinguish physical from virtual to prevent the silent bugs that arise from mixing them:

```c
typedef uint64_t phys_addr_t;  // physical address
typedef uint64_t virt_addr_t;  // virtual address
```

### Function Prefixes

Every function is prefixed with the abbreviation of its owning module, making the origin of a call immediately apparent at every use site:

```c
mm_pmm_init(...)              // memory management — PMM
kprintf(...)                  // kio — kernel output
drivers_console_putc(...)     // drivers — console
arch_serial_write(...)        // arch layer — serial
mmu_map_mmio(...)             // page_tables — MMU
```

### Type-Scoped Function Prefixes

When a function's primary subject is a specific type exported by the module, extend the prefix to include the type name. This makes call sites self-documenting without requiring the reader to inspect the function signature:

```c
// preferred — the call site names the type being operated on
gfx_surface_put_pixel(&surface, x, y, color);
gfx_surface_fill_rect(&surface, x, y, w, h, color);
gfx_surface_init(&surface, base, width, height, pitch, bpp, r, g, b);

// avoid — leaves the call site ambiguous about what the first argument is
gfx_put_pixel(&surface, x, y, color);
```

The general pattern is `<module>_<type>_<action>` for operations whose primary subject is a named type, and `<module>_<action>` when the module has a single implicit subject or the operation is not type-specific.

### File Naming

A file must be named after its **responsibility**, not its containing module. A file `kio/kio.c` carries no more information than its directory path already does — name it for what it does: `kio/output.c`. The same applies to public headers: `<kio/output.h>` states what it contains; `<kio/kio.h>` does not.

Some examples:

| Module | Avoid | Prefer |
|--------|-------|--------|
| `kio` | `kio/kio.c`, `<kio/kio.h>` | `kio/output.c`, `<kio/output.h>` |
| `interrupts` | `interrupts/interrupts.c` | `interrupts/init.c` |
| `gfx` | `gfx/gfx.c` | `gfx/draw.c`, `gfx/surface.h` |

**Exception — contracts:** contract headers are intentionally named after the contract itself (`contracts/memmap.h`, `contracts/display.h`) because the header *is* the named type definition. The module and the responsibility are one and the same in this case.
