# TinyOS Naming Conventions and Terminology

## Core Terminology Philosophy

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
| Tree | **Binary Tree** | Be specific about the tree type |

### Memory Management
| Instead of... | We use... | Rationale |
|---------------|-----------|-----------|
| Memory Page | **Page Frame** | Distinguishes physical memory management |
| Memory Space | **Address Space** | Emphasizes MMU and virtual memory |
| Pointer | **Address** (when appropriate) | Clarifies memory addressing |

### Hardware Interfaces
| Instead of... | We use... | Rationale |
|---------------|-----------|-----------|
| Device Communication | **Port I/O** | Specifies hardware I/O mechanism |
| Event Handler | **Interrupt Vector** | Describes hardware interrupt table |
| Variable | **Register** | When referring to CPU registers |

### System Concepts
| Instead of... | We use... | Rationale |
|---------------|-----------|-----------|
| Process | **Task** (initially) | Simpler concept for basic scheduler |
| Thread | **Execution Context** | Explicit about CPU state |
| Mutex | **Spinlock** | Describes actual implementation |

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

#### Library Prefix: `libk*`
All kernel libraries use the `libk` prefix to:
- Distinguish from user-space libraries
- Prevent naming conflicts with standard C
- Indicate bare-metal, kernel-space code

```
lib/
├── libkbuffer/    # Character buffer operations
├── libkio/        # Input/Output operations
├── libkstd/       # Standard library subset
└── libkarch/      # Architecture abstraction (future)
```

#### Core Library Descriptions

**`libkbuffer`** - Character Buffer Operations
- Character buffer manipulation and views
- Buffer slicing and substring operations
- Memory-safe buffer operations
- Fixed-size and dynamic buffer management
- Buffer comparison and searching
- Character encoding/decoding operations

**`libkio`** - Input/Output Operations
- Formatted output (printf family)
- Character and buffer I/O
- Debug output and logging
- Binary data serialization
- Stream abstractions for devices

**`libkstd`** - Standard Library Subset
- Basic type definitions (stdint.h, stddef.h)
- Essential macros and constants
- Compiler intrinsics wrappers
- Platform-independent abstractions

### Function Naming

#### Module Prefixes
Functions are prefixed with their module abbreviation:

```c
// libkbuffer functions (prefix: kbuf_)
char_buffer_t* kbuf_create(size_t capacity);
void kbuf_append(char_buffer_t* buf, const char* data);
void kbuf_destroy(char_buffer_t* buf);
char_buffer_view_t kbuf_slice(char_buffer_t* buf, size_t start, size_t len);

// libkio functions (prefix: kio_)
void kio_printf(const char* format, ...);
void kio_write_buffer(const char_buffer_t* buf);
void kio_debug_hex_dump(const void* data, size_t len);

// Architecture functions (prefix: arch_)
void arch_enable_interrupts(void);
void arch_disable_interrupts(void);
phys_addr_t arch_virt_to_phys(virt_addr_t vaddr);
void arch_flush_tlb(void);

// Memory management functions (prefix: mm_)
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);
void mm_unmap_page(virt_addr_t vaddr);
page_frame_t* mm_alloc_page_frame(void);
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