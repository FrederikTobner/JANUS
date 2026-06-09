# Conventions

## Coding Style

### Multi-Dimensional Memory Access

Prefer explicit pointer arithmetic over array indexing for multi-dimensional
structures. The intent is to make memory layout and stride visible at the point
of use:

```c
// Preferred: layout and stride are explicit
u32 * pixel_ptr = framebuffer + (y * pitch) + x;
*pixel_ptr = color;

// Avoid: the pitch is hidden inside the subscript operator
u32 color = framebuffer[y][x];
```

For simple one-dimensional arrays, standard subscript notation is fine.

### Structure Encapsulation

Define structures publicly by default. Public structures allow stack allocation,
embedding, and full debugger visibility. Reserve opaque handles for the rare case
where the underlying representation genuinely varies at runtime (e.g. framebuffer
backends, filesystem drivers).

### East-Const

`const` is placed east of the type throughout the codebase:

```c
char const * message;         // pointer to const char
boot_context_t const * ctx;   // pointer to const boot_context_t
```

### Documentation

Every public structure in a public header carries a Doxygen `@brief` comment
explaining its fields, invariants, and layout constraints.

### Static Function Layout

In every `.c` file, the sections appear in this order:

1. `static` forward declarations — shows what internal helpers exist
2. Public function definitions — the externally visible API
3. `static` function definitions — the implementations of the helpers

## Naming

### Implementation-Specific Types

Use implementation-specific names rather than abstract ones:

| Instead of | Use | Rationale |
|---|---|---|
| String | Character buffer | It is a buffer of characters in memory |
| Map | Hashtable | Names the backing implementation |
| Queue | Ring buffer | Names the data structure explicitly |
| List | Linked list | Makes traversal cost apparent |
| Vector | Dynamic array | Names the backing implementation |

### Type Suffixes

All custom types carry the `_t` suffix. Structure tags match the typedef name
without the suffix:

```c
typedef struct linked_list linked_list_t;
typedef struct ring_buffer ring_buffer_t;
```

Memory management types distinguish physical from virtual:

```c
typedef uint64_t phys_addr_t;  // physical address
typedef uint64_t virt_addr_t;  // virtual address
```

### Function Prefixes

Functions are prefixed with their module abbreviation so the owning module is
apparent at every call site:

```c
mm_pmm_init(...)           // memory management — PMM
kprintf(...)               // kio — kernel output
drivers_tty_putc(...)      // drivers — TTY
arch_serial_write(...)     // arch layer — serial
mmu_map_mmio(...)          // page_tables — MMU
```

## Terminology

| Term | Meaning |
|---|---|
| Subsystem | An independent kernel module under `kernel/subsys/` |
| Core | A shared service module under `kernel/core/` |
| Protocol library | A boot-protocol-specific static library (`boot_limine`, `boot_multiboot2`) |
| Contract | A shared type definition crossing subsystem boundaries, with an authorised consumer allowlist |
| HHDM | Higher Half Direct Map — Limine maps all physical RAM at a fixed virtual offset |
| phys_addr_t | A physical address value |
| virt_addr_t | A virtual address value |
