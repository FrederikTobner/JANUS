# Terminology

JANUS uses precise, implementation-aware terminology throughout the codebase. Naming is not treated as cosmetic — the words chosen for types, functions, and modules carry technical meaning and should reflect the actual data structure or hardware concept involved, not a high-level abstraction borrowed from another language ecosystem.

## Guiding Principle

If you call a hashtable a "map," you obscure the implementation. If you call a character buffer a "string," you import assumptions from managed languages where strings carry metadata, encoding guarantees, and reference semantics. In a freestanding C kernel, none of that applies. Name things for what they are.

## Data Structures

Use implementation-specific names rather than abstract ones:

| Instead of | Use             | Rationale |
|------------|-----------------|-----------|
| String     | Character buffer| It is a buffer of characters in memory, with an explicit length and capacity. |
| Map        | Hashtable       | The backing implementation is a hashtable; name it accordingly. |
| Queue      | Ring buffer     | A circular buffer with head and tail pointers. |
| List       | Linked list     | A pointer-based structure with explicit traversal cost. |
| Binary Tree| Red-black tree  | Be specific about the balancing scheme. |
| Vector     | Dynamic array   | An array whose capacity changes at runtime. |

## Type Naming

All custom types carry the `_t` suffix. Structure tags match the typedef name without the suffix:

```c
// Good: implementation-based naming
typedef struct linked_list linked_list_t;
typedef struct ring_buffer ring_buffer_t;
typedef struct hashtable hashtable_t;

// Bad: abstract, high-level naming
typedef struct list list_t;
typedef struct queue queue_t;
typedef struct map map_t;
```

Memory management types distinguish physical from virtual and kernel from user:

```c
typedef struct page_frame page_frame_t;           // Physical memory frame
typedef struct virt_addr_space virt_addr_space_t; // Virtual address space
typedef uint64_t phys_addr_t;                    // Physical address
typedef uint64_t virt_addr_t;                    // Virtual address
typedef struct kmalloc_block kmalloc_block_t;     // Kernel heap allocation
```

Hardware interface types use the exact terminology from processor manuals. This convention applies exclusively to types in the `arch/` layer:

```c
// x86_64 (Intel/AMD manual terminology)
typedef struct page_table_entry pte_t;
typedef struct page_directory_entry pde_t;
typedef struct global_descriptor_table gdt_t;
typedef struct interrupt_descriptor_table idt_t;
typedef struct task_state_segment tss_t;

// Generic hardware interfaces
typedef struct port_io port_io_t;
typedef struct mmio_region mmio_region_t;
typedef struct irq_handler irq_handler_t;
typedef struct cpu_regs cpu_regs_t;
```

## Module and Library Naming

Kernel libraries are organised under `lib/`, each with a simple name describing its purpose:

```text
lib/
├── types/      # Basic types and compiler attributes (header-only)
├── memory/     # Memory operations (memcpy, memset, etc.)
├── buffer/     # Character buffer operations
└── fio/        # Formatted I/O operations
```

**`types`** provides fixed-width integer types, boolean macros, compiler attributes (`__packed`, `__noreturn`, `likely`/`unlikely`), and essential macros (`NULL`, `ARRAY_SIZE`, `container_of`). It is header-only.

**`memory`** provides low-level memory manipulation (`memcpy`, `memmove`, `memset`, `memcmp`, `memzero`) with no libc dependency.

**`buffer`** provides character buffer manipulation, views, slicing, and fixed-size or dynamic buffer management.

**`fio`** provides formatted output (`printf` family), character and buffer I/O, debug output, and stream abstractions for devices.

## Function Naming

Functions are prefixed with their module abbreviation. The prefix makes the owning module immediately apparent at every call site:

```c
// buffer library (prefix: buf_)
buf_create(capacity);
buf_append(&buf, data);
buf_destroy(&buf);
buf_slice(&buf, start, len);

// formatted I/O (prefix: fio_)
fio_printf(format, ...);
fio_write_buffer(&buf);
fio_debug_hex_dump(data, len);

// memory library — retains standard names
memcpy(dest, src, n);
memset(dest, value, n);
memcmp(s1, s2, n);

// architecture layer (prefix: arch_)
arch_enable_interrupts();
arch_disable_interrupts();
arch_virt_to_phys(vaddr);
arch_flush_tlb();

// memory management (prefix: mm_)
mm_map_page(vaddr, paddr, flags);
mm_unmap_page(vaddr);
mm_alloc_page_frame();
```

### Operation Clarity

Function names must clearly indicate what they do, what they modify, and what they return:

```c
// Allocation: allocates and returns
kmalloc_block_t *kmalloc_alloc(size_t size);
void kmalloc_free(kmalloc_block_t *block);

// Hardware interaction: reads, no side effects
uint8_t port_read_byte(uint16_t port);
void port_write_byte(uint16_t port, uint8_t value);

// Data modification: makes the mutation explicit
void buf_zero_fill(char_buffer_t *buf);
char_buffer_view_t buf_readonly_view(char_buffer_t const *buf);
```

### Global Variables

Global variables are prefixed with their module name. Truly global state uses the `g_` prefix:

```c
static hashtable_t *kmalloc_cache_table;
static size_t mm_total_frames;

volatile uint64_t g_system_tick_count;
```

## Hardware Terminology

When referring to hardware features in architecture-specific code, use the exact terms from the relevant processor manual:

- **x86_64 (Intel/AMD):** Page Table Entry (PTE), Control Register (CR0, CR3), Model-Specific Register (MSR), Interrupt Descriptor Table (IDT).
- **aarch64 (ARM):** Translation Table Entry (TTE), Exception Vector Table (EVT), System Control Register (SCTLR).

This convention applies only to the `arch/` layer. Generic kernel code uses architecture-neutral names.
