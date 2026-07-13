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
drivers_serial_putc(...)      // Generic serial driver, putc function
arch_serial_write(...)        // Arch specific serial driver write implementation
mmu_map_mmio(...)             // page_tables — MMU
```

### File Naming

A file must be named after its **responsibility**, not its containing module.
A file `kio/kio.c` carries no more information than its directory path already does, so split it into two parts in order to name it for what it does: `kio/output.c` and `kio/input.c` for example.
The same applies to public headers: `<kio/output.h>` states what it contains; `<kio/kio.h>` does not.

**Exception — contracts:** contract headers are intentionally named after the contract itself (`contracts/memmap.h`, `contracts/display.h`) because the header *is* the named type definition. The module and the responsibility are one and the same in this case.
