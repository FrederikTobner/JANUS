# TinyOS Naming Conventions and Terminology

## Core Terminology Philosophy

> "The cheapest, fastest, and most reliable components are those that aren't there."
> — *Gordon Bell*

**Use the right damn words.** TinyOS uses explicit, hardware-aware terminology that reflects actual implementation, not fluffy abstractions.

If you call a hashtable a "map," you're hiding the implementation. If you call a character buffer a "string," you're importing assumptions from high-level languages. **Words matter. Use precise ones.**

## Preferred Technical Terms

### Data Structures

**Don't use vague high-level bullshit. Use implementation-specific terms.**

| Instead of... | We use... | Why |
|---------------|-----------|-----|
| String | **Character Buffer** | It's a buffer of characters in memory, not some magical string object |
| Map | **Hashtable** | We're using a hashtable. Say so. |
| Queue | **Ring Buffer** | It's a circular buffer with head/tail pointers |
| List | **Linked List** | Pointer-based structure with explicit traversal cost |
| Binary Tree | **Red-black Tree** | Be specific about the tree type |
| Vector | **Dynamic Array** | An array that can has a dynamic capacity, changing at runtime |

## Naming Conventions

### Type Naming

#### Suffix Convention: `_t`
All custom types use the `_t` suffix:
```c
typedef struct hashtable hashtable_t;
typedef struct char_buffer char_buffer_t;
typedef struct page_frame page_frame_t;
typedef uint64_t phys_addr_t;
```

#### Structure Names
Use descriptive, implementation-specific names:
```c
// Good: Implementation-based naming
typedef struct linked_list linked_list_t;
typedef struct ring_buffer ring_buffer_t;
typedef struct hashtable hashtable_t;
typedef struct binary_tree binary_tree_t;

// Bad: Abstract, high-level naming
typedef struct list list_t;
typedef struct queue queue_t;
typedef struct map map_t;
```

#### Memory Management Types
Be explicit about physical vs virtual, kernel vs user:
```c
typedef struct page_frame page_frame_t;           // Physical memory frame
typedef struct virt_addr_space virt_addr_space_t; // Virtual address space
typedef uint64_t phys_addr_t;                    // Physical address
typedef uint64_t virt_addr_t;                    // Virtual address
typedef struct kmalloc_block kmalloc_block_t;     // Kernel heap allocation
```

#### Hardware Interface Types
Use exact terminology from processor manuals:
```c
// x86-64 specific (Intel/AMD manual terminology)
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

### Module and Library Naming

#### Kernel Library Structure
Kernel libraries are organized under `lib/` without a common prefix:
- Simple, focused names that describe their purpose
- No `libk` prefix (the `lib/` directory already indicates library code)
- Each library is self-contained and focused

```
lib/
├── types/      # Basic types and compiler attributes (header-only)
├── memory/     # Memory operations (memcpy, memset, etc.)
├── buffer/     # Character buffer operations  
└── fio/        # Formatted I/O operations
```

#### Core Library Descriptions

**`types`** - Basic Types and Compiler Attributes (Header-Only)
- Fixed-width integer types (uint8_t, int32_t, etc.)
- Boolean type and macros
- Compiler attributes (__packed, __noreturn, likely/unlikely)
- Essential macros (NULL, ARRAY_SIZE, container_of)
- Platform-independent type definitions

**`memory`** - Memory Operations
- Low-level memory manipulation (memcpy, memmove, memset)
- Memory comparison (memcmp)
- Memory zeroing (memzero)
- No libc dependency

**`buffer`** - Character Buffer Operations
- Character buffer manipulation and views
- Buffer slicing and substring operations
- Memory-safe buffer operations
- Fixed-size and dynamic buffer management
- Buffer comparison and searching

**`fio`** - Formatted I/O Operations
- Formatted output (printf family)
- Character and buffer I/O
- Debug output and logging
- Binary data serialization
- Stream abstractions for devices

### Function Naming

#### Module Prefixes
Functions are prefixed with their module abbreviation:

```c
// buffer library functions (prefix: buf_)
buf_create(size_t capacity);
void buf_append(buffer_t* buf, char const * data);
void buf_destroy(buffer_t* buf);
buf_view_t buf_slice(buffer_t* buf, size_t start, size_t len);

// fio library functions (prefix: fio_)
void fio_printf(char const * format, ...);
void fio_write_buffer(buffer_t const * buf);
void fio_debug_hex_dump(void const * data, size_t len);

// memory library functions (prefix: none - standard names)
void * memcpy(void * dest, void const * src, size_t n);
void * memset(void * dest, int value, size_t n);
int memcmp(void const * s1, void const * s2, size_t n);

// Architecture functions (prefix: arch_)
void arch_enable_interrupts(void);
void arch_disable_interrupts(void);
phys_addr_t arch_virt_to_phys(virt_addr_t vaddr);
void arch_flush_tlb(void);

// Memory management functions (prefix: memman_)
int memman_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);
void memman_unmap_page(virt_addr_t vaddr);
page_frame_t* memman_alloc_page_frame(void);
```

#### Operation Clarity
Function names must clearly indicate:
1. **What they do** - The operation performed
2. **What they modify** - Any side effects
3. **What they return** - Return value semantics

```c
// Clear about memory allocation
kmalloc_block_t* kmalloc_alloc(size_t size);          // Allocates and returns
void kmalloc_free(kmalloc_block_t* block);            // Deallocates

// Clear about hardware interaction  
uint8_t port_read_byte(uint16_t port);                // Reads, no side effects
void port_write_byte(uint16_t port, uint8_t value);   // Writes to hardware

// Clear about data modification
void kbuf_zero_fill(char_buffer_t* buf);              // Modifies buffer
void kbuf_append(char_buffer_t* buf, const char* data); // Modifies buffer
char_buffer_view_t kbuf_readonly_view(const char_buffer_t* buf); // Read-only view
```

### Variable Naming

#### Local Variables
Use descriptive names, avoid abbreviations unless common:
```c
// Good
page_frame_t* frame;
size_t buffer_size;
uint64_t physical_address;

// Acceptable abbreviations
int i, j, k;           // Loop counters
char* ptr;             // Pointer
size_t len;            // Length
uint64_t addr;         // Address
```

#### Global Variables
Prefix with module name or `g_`:
```c
// Module-prefixed
static hashtable_t* kmalloc_cache_table;
static size_t mm_total_frames;

// Or g_ prefix for truly global
volatile uint64_t g_system_tick_count;
```

#### Constants and Macros
Use UPPER_CASE with underscores:
```c
#define KERNEL_VIRTUAL_BASE    0xFFFFFFFF80000000
#define PAGE_SIZE              4096
#define MAX_IRQ_HANDLERS       256

// Processor-specific constants from manuals
#define X86_64_CR0_PE          (1 << 0)    // Protection Enable
#define X86_64_CR0_PG          (1 << 31)   // Paging
```

### File Naming

#### Source Files
Use lowercase with underscores:
```
buffer.c
hashtable.c
page_frame.c
interrupt_vector.c
```

#### Header Files
Match corresponding source file:
```
buffer.h
hashtable.h
page_frame.h
interrupt_vector.h
```

#### Architecture-Specific Files
Include architecture in name:
```
arch/x86_64/cpu.c
arch/x86_64/io.c
arch/x86_64/interrupt.c
```

## Terminology Consistency

### Always Use Exact Terms

When referring to hardware features, use the **exact** terminology from processor manuals:

**x86-64 (Intel/AMD)**
- Page Table Entry (PTE), not "page descriptor"
- Control Register (CR0, CR3), not "control variable"
- Model-Specific Register (MSR), not "CPU register"
- Interrupt Descriptor Table (IDT), not "interrupt table"

**Future ARM**
- Translation Table Entry (TTE)
- Exception Vector Table (EVT)
- System Control Register (SCTLR)

### Avoid High-Level Abstractions

Prefer technical terms that expose implementation:
- **Hashtable** over "Map" or "Dictionary"
- **Character Buffer** over "String"
- **Ring Buffer** over "Queue" (when circular)
- **Linked List** over "List"
- **Page Frame** over "Page" (for physical memory)
- **Address Space** over "Memory Space"
