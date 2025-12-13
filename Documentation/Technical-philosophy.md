# TinyOS Technical Philosophy and Naming Conventions

## Core Technical Philosophy

### 1. Low-Level, Precise Terminology
TinyOS embraces explicit, hardware-aware terminology that reflects the actual implementation rather than high-level abstractions.

**Preferred Technical Terms:**
- **Character Buffer** instead of "String" - reflects the underlying memory structure
- **Hashtable** instead of "Map" - describes the actual data structure algorithm  
- **Page Frame** instead of "Memory Page" - indicates physical memory management
- **Port I/O** instead of "Device Communication" - specifies the hardware mechanism
- **Interrupt Vector** instead of "Event Handler" - describes the hardware table
- **Register** instead of "Variable" - when referring to CPU registers
- **Address Space** instead of "Memory Space" - emphasizes memory management unit

**Rationale:** C developers work close to hardware and benefit from terminology that directly maps to underlying mechanisms. This reduces abstraction overhead and makes the codebase more educational.

### 2. Module Naming Philosophy

#### Library Prefix Convention: `libk*`
All kernel libraries use the `libk` prefix to:
- **Distinguish** kernel libraries from user-space libraries
- **Prevent** naming conflicts with standard C libraries
- **Indicate** bare-metal, kernel-space implementations

#### Core Library Categories

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

**`libkarch`** - Architecture Abstraction
- CPU-specific operations
- Memory management unit abstractions
- Interrupt handling interfaces
- Platform-specific I/O operations

### 3. Data Structure Naming

#### Container Types
```c
// Use implementation-based names
typedef struct hashtable hashtable_t;        // NOT: map_t
typedef struct linked_list linked_list_t;    // NOT: list_t  
typedef struct ring_buffer ring_buffer_t;    // NOT: queue_t
typedef struct binary_tree binary_tree_t;    // NOT: tree_t
typedef struct char_buffer char_buffer_t;    // NOT: string_t
```

#### Memory Management
```c
// Explicit about memory layout and management
typedef struct page_frame page_frame_t;      // Physical memory frame
typedef struct virt_addr_space virt_addr_space_t; // Virtual address space
typedef struct phys_addr phys_addr_t;        // Physical address
typedef struct kmalloc_block kmalloc_block_t; // Kernel heap block
```

#### Hardware Interface Types
```c
// Hardware-specific naming
typedef struct port_io port_io_t;            // I/O port operations
typedef struct mmio_region mmio_region_t;    // Memory-mapped I/O
typedef struct irq_handler irq_handler_t;    // Interrupt request handler
typedef struct cpu_regs cpu_regs_t;          // CPU register state
```

### 4. Function Naming Conventions

#### Module Prefixes
Functions are prefixed with their module abbreviation:
```c
// libkbuffer functions
char_buffer_t* kbuf_create(size_t capacity);
void kbuf_append(char_buffer_t* buf, const char* data);
char_buffer_view_t kbuf_slice(char_buffer_t* buf, size_t start, size_t len);

// libkio functions  
void kio_printf(const char* format, ...);
void kio_write_buffer(const char_buffer_t* buf);
void kio_debug_hex_dump(const void* data, size_t len);

// Architecture functions
void arch_enable_interrupts(void);
phys_addr_t arch_virt_to_phys(virt_addr_t vaddr);
void arch_flush_tlb(void);
```

#### Operation Clarity
Function names should clearly indicate their operation and any side effects:
```c
// Clear about memory allocation
kmalloc_block_t* kmalloc_alloc(size_t size);          // Allocates memory
void kmalloc_free(kmalloc_block_t* block);            // Frees memory  

// Clear about hardware interaction
uint8_t port_read_byte(uint16_t port);                // Reads from I/O port
void port_write_byte(uint16_t port, uint8_t value);   // Writes to I/O port

// Clear about data modification
void kbuf_zero_fill(char_buffer_t* buf);              // Modifies buffer
char_buffer_view_t kbuf_readonly_view(const char_buffer_t* buf); // Read-only
```

### 5. File and Directory Structure Philosophy

#### Flat Module Hierarchy
```
TinyOS/
├── kernel/           # Core kernel - no subdirs for simple components
├── boot/            # Boot components - architecture-agnostic parts
├── arch/            # Architecture-specific - subdivided by ISA
│   └── x86_64/     # Specific architecture implementation
├── lib/             # Utility libraries - each as separate module
│   ├── libkbuffer/ # Character buffer operations
│   ├── libkio/     # I/O operations  
│   └── libkstd/    # Standard library subset
├── mm/              # Memory management - complex enough for own module
└── drivers/         # Device drivers - subdivided by device type
```

#### Include Path Philosophy
```c
// Global includes - cross-cutting concerns
#include <tinyos/types.h>        // Global type definitions
#include <tinyos/config.h>       // Build configuration

// Module includes - specific functionality  
#include <lib/buffer.h>          // From libkbuffer
#include <lib/io.h>             // From libkio
#include <arch/cpu.h>           // From libkarch

// Local includes - internal to module
#include "kernel/internal.h"     // Kernel-internal interfaces
```

### 6. Hardware Abstraction Philosophy

#### Direct Hardware Terminology
When interfacing with hardware, use the exact terminology from processor manuals:

```c
// x86-64 specific terms from Intel/AMD manuals
typedef struct page_table_entry pte_t;
typedef struct page_directory_entry pde_t;  
typedef struct global_descriptor_table gdt_t;
typedef struct interrupt_descriptor_table idt_t;
typedef struct task_state_segment tss_t;

// ARM specific terms (future)  
typedef struct translation_table_entry tte_t;
typedef struct exception_vector_table evt_t;
```

#### Abstraction Layers
```c
// Low-level hardware interface (arch-specific)
void x86_64_write_cr3(uint64_t pml4_phys);

// Mid-level abstraction (arch-agnostic interface)  
void arch_set_page_table_root(phys_addr_t page_table);

// High-level kernel interface
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);
```

### 7. Error Handling Philosophy

#### Explicit Error Codes
Use explicit, technical error codes rather than generic ones:

```c
typedef enum {
    KERNEL_SUCCESS = 0,
    KERNEL_ERROR_INVALID_ADDRESS,    // NOT: KERNEL_ERROR_BAD_INPUT
    KERNEL_ERROR_PAGE_FAULT,         // NOT: KERNEL_ERROR_MEMORY  
    KERNEL_ERROR_PROTECTION_VIOLATION,
    KERNEL_ERROR_DOUBLE_FAULT,
    KERNEL_ERROR_INVALID_OPCODE,
    KERNEL_ERROR_DIVIDE_BY_ZERO
} kernel_error_t;
```

### 8. Documentation Philosophy

#### Technical Precision
Comments and documentation should:
- Reference specific processor manuals and standards
- Include relevant bit patterns and register layouts
- Explain hardware constraints and timing requirements
- Provide rationale for architectural decisions

```c
/**
 * Initialize the x86-64 Global Descriptor Table
 * 
 * Sets up a flat memory model as required by x86-64 long mode.
 * Reference: Intel SDM Vol 3A Section 3.4.5
 * 
 * @param gdt Pointer to GDT structure (must be 8-byte aligned)
 * @return KERNEL_SUCCESS or error code
 */
kernel_error_t x86_64_init_gdt(gdt_t* gdt);
```

This philosophy ensures TinyOS remains true to its educational mission while maintaining professional-grade precision in its terminology and structure.