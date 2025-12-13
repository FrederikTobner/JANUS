# TinyOS API Design Guidelines

## Function Design Principles

### 1. Clear Intent

**Function names that don't clearly state what they do are bad names.**

Your function must communicate:
- What it does (the action)
- What it modifies (side effects)
- What it returns (return value meaning)

If someone has to read the implementation to understand what your function does, you named it wrong.

### 2. Consistent Prefixing
All functions use their module prefix:

| Module | Prefix | Example |
|--------|--------|---------|
| libkbuffer | `kbuf_` | `kbuf_create()` |
| libkio | `kio_` | `kio_printf()` |
| arch | `arch_` | `arch_enable_interrupts()` |
| mm | `mm_` | `mm_map_page()` |
| kernel | `kernel_` | `kernel_panic()` |

### 3. Verb-Noun Pattern
Functions follow verb-noun or verb-object pattern:
```c
kbuf_create()         // create a buffer
kbuf_destroy()        // destroy a buffer
mm_map_page()         // map a page
arch_flush_tlb()      // flush the TLB
port_read_byte()      // read a byte from port
```

## Function Naming Patterns

### Creation and Destruction
```c
// Allocation/creation functions return pointer
char_buffer_t* kbuf_create(size_t capacity);
hashtable_t* hashtable_create(size_t initial_size);

// Destruction functions take pointer
void kbuf_destroy(char_buffer_t* buffer);
void hashtable_destroy(hashtable_t* table);

// In-place initialization (no allocation)
void kbuf_init(char_buffer_t* buffer, char* data, size_t size);
void kbuf_cleanup(char_buffer_t* buffer);
```

### Getters and Setters
```c
// Get operations - no side effects
size_t kbuf_get_length(char_buffer_t const * buffer);
uint64_t arch_get_cr3(void);

// Set operations - explicit modification
void kbuf_set_length(char_buffer_t* buffer, size_t new_length);
void arch_set_cr3(uint64_t value);
```

### Boolean Queries
Use `is_`, `has_`, `can_` prefixes:
```c
bool mm_is_page_present(virt_addr_t vaddr);
bool kbuf_has_capacity(char_buffer_t const * buf, size_t required);
bool arch_can_enable_feature(uint32_t feature);
```

### Data Modification
Make side effects explicit:
```c
// Clearly modifies buffer
void kbuf_append(char_buffer_t* buffer, char const * data);
void kbuf_zero_fill(char_buffer_t* buffer);
void kbuf_truncate(char_buffer_t* buffer, size_t new_length);

// Read-only operations (return views/copies)
char_buffer_view_t kbuf_slice(char_buffer_t const * buf, size_t start, size_t len);
char_buffer_view_t kbuf_readonly_view(char_buffer_t const * buf);
```

### Hardware Interaction
Be explicit about I/O operations:
```c
uint8_t port_read_byte(uint16_t port);
uint16_t port_read_word(uint16_t port);
uint32_t port_read_dword(uint16_t port);

void port_write_byte(uint16_t port, uint8_t value);
void port_write_word(uint16_t port, uint16_t value);
void port_write_dword(uint16_t port, uint32_t value);

// Memory-mapped I/O
uint32_t mmio_read_32(mmio_region_t const * region, size_t offset);
void mmio_write_32(mmio_region_t* region, size_t offset, uint32_t value);
```

## Parameter Conventions

### Parameter Order

**Consistent parameter ordering is not optional.**

1. **Input parameters** (const pointers, values to read)
2. **Output parameters** (non-const pointers to write)
3. **Size/length parameters**
4. **Optional flags/options**

```c
// Good parameter ordering
int kbuf_copy(
    char_buffer_t const * source,      // Input (1)
    char_buffer_t       * destination, // Output (2)
    size_t                max_bytes,   // Size (3)
    uint32_t              flags        // Flags (4)
);

void mm_map_range(
    virt_addr_t vaddr_start,          // Input (1)
    phys_addr_t paddr_start,          // Input (1)
    page_frame_t* frames,             // Output (2)
    size_t page_count,                // Size (3)
    uint32_t flags                    // Flags (4)
);
```

### Const Correctness
Always use const for read-only parameters:
```c
// Good
size_t kbuf_length(char_buffer_t const * buffer);
void kbuf_print(char_buffer_t const * buffer);

// Bad - missing const
size_t kbuf_length(char_buffer_t* buffer);
```

### Pointer vs Value
- **Structures**: Pass by pointer (const for read-only)
- **Small types** (int, size_t, pointers): Pass by value
- **Output parameters**: Always pass by pointer

```c
// Structures by pointer
void kbuf_append_buffer(char_buffer_t* dest, char_buffer_t const * source);

// Small types by value
uint8_t port_read_byte(uint16_t port);
bool mm_is_aligned(uint64_t address, size_t alignment);

// Output by pointer
int mm_get_page_flags(virt_addr_t vaddr, uint32_t* flags_out);
```

## Return Value Conventions

### Success/Failure Indication
```c
// Option 1: Integer return codes
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);
// Returns: 0 on success, negative error code on failure

// Option 2: Boolean for simple success/failure  
bool kbuf_append(char_buffer_t* buffer, char const * data);
// Returns: true on success, false on failure

// Option 3: Pointer (NULL indicates failure)
page_frame_t* mm_alloc_page_frame(void);
// Returns: Valid pointer on success, NULL on failure
```

### Error Code Convention
Use negative values for errors:
```c
#define KERNEL_SUCCESS                    0
#define KERNEL_ERROR_INVALID_ADDRESS     -1
#define KERNEL_ERROR_OUT_OF_MEMORY       -2
#define KERNEL_ERROR_PERMISSION_DENIED   -3
#define KERNEL_ERROR_PAGE_FAULT          -4

int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags) {
    if (!mm_is_aligned(vaddr, PAGE_SIZE)) {
        return KERNEL_ERROR_INVALID_ADDRESS;
    }
    // ... do work ...
    return KERNEL_SUCCESS;
}
```

### Multiple Return Values
Use output parameters for additional return values:
```c
/**
 * Get page flags and physical address
 * 
 * @param vaddr Virtual address to query
 * @param paddr_out Output: Physical address (can be NULL)
 * @param flags_out Output: Page flags (can be NULL)
 * @return 0 on success, negative error code if page not mapped
 */
int mm_get_page_info(
    virt_addr_t vaddr,
    phys_addr_t* paddr_out,
    uint32_t* flags_out
);

// Usage
phys_addr_t paddr;
uint32_t flags;
if (mm_get_page_info(vaddr, &paddr, &flags) == 0) {
    // Success - use paddr and flags
}
```

## Error Handling

### Explicit Error Codes
Define module-specific error codes:
```c
// mm/include/mm/errors.h
typedef enum {
    MM_SUCCESS = 0,
    MM_ERROR_INVALID_ADDRESS,
    MM_ERROR_PAGE_NOT_PRESENT,
    MM_ERROR_PAGE_ALREADY_MAPPED,
    MM_ERROR_OUT_OF_FRAMES,
    MM_ERROR_PROTECTION_VIOLATION
} mm_error_t;
```

### Error Propagation
```c
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags) {
    // Validate parameters
    if (!mm_is_page_aligned(vaddr)) {
        return MM_ERROR_INVALID_ADDRESS;
    }
    
    // Call lower-level function
    int result = arch_map_page_low_level(vaddr, paddr, flags);
    if (result != 0) {
        return result;  // Propagate error
    }
    
    return MM_SUCCESS;
}
```

### Panic vs Error Return
```c
// Use panic for unrecoverable errors
void kernel_panic(char const * message) __attribute__((noreturn));

// Return errors for recoverable situations
int mm_map_page(...) {
    if (critical_invariant_violated) {
        kernel_panic("Memory management invariant violated");
    }
    if (recoverable_error) {
        return MM_ERROR_OUT_OF_FRAMES;  // Caller can handle this
    }
    return MM_SUCCESS;
}
```

## API Stability and Versioning

### Internal vs External APIs
- **Internal APIs**: Can change freely within a module
- **External APIs**: Require careful consideration of compatibility

### API Evolution
When changing public APIs:
1. Deprecate old function (keep it working)
2. Add new function with `_v2` suffix
3. Update documentation
4. Provide migration guide
5. Remove deprecated function in next major version

```c
// Old API (deprecated)
__attribute__((deprecated)) 
void kbuf_append(char_buffer_t* buf, char* data);

// New API (improved)
bool kbuf_append_safe(char_buffer_t* buf, char const * data, size_t len);
```

## Documentation Requirements

### Function Documentation Template
```c
/**
 * [Brief one-line description]
 * 
 * [Detailed explanation of what the function does, including any
 * important behavior, side effects, or limitations]
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * 
 * @note Important notes about usage
 * @warning Warnings about potential pitfalls
 * @see Related functions
 * 
 * Reference: [Processor manual section if applicable]
 * 
 * Example:
 * @code
 * char_buffer_t* buf = kbuf_create(256);
 * kbuf_append(buf, "Hello");
 * kbuf_destroy(buf);
 * @endcode
 */
char_buffer_t* kbuf_create(size_t capacity);
```

### Hardware-Specific Functions
```c
/**
 * Write to Control Register 3 (CR3) - Page Table Base Address
 * 
 * Loads the physical address of the top-level page table structure
 * into CR3. This triggers a TLB flush on x86-64.
 * 
 * @param pml4_phys Physical address of PML4 table (must be 4KB-aligned)
 * 
 * @warning This function flushes the entire TLB. Use selectively.
 * @note Address must be in lower 52 bits (x86-64 physical address limit)
 * 
 * Reference: Intel SDM Vol 3A Section 2.5 (Control Registers)
 */
void arch_write_cr3(phys_addr_t pml4_phys);
```

## Testing Considerations

### Testable API Design
```c
// Good: Easy to test with different inputs
int mm_map_page(virt_addr_t vaddr, phys_addr_t paddr, uint32_t flags);

// Bad: Hard to test (depends on global state)
void setup_everything_for_kernel(void);
```

### Interface Segregation
Split large interfaces into focused ones:
```c
// Instead of one monolithic interface
typedef struct memory_manager {
    int (*map)(virt_addr_t, phys_addr_t, uint32_t);
    int (*unmap)(virt_addr_t);
    page_frame_t* (*alloc)();
    void (*free)(page_frame_t*);
    // ... 20 more functions
} memory_manager_t;

// Use separate, focused interfaces
typedef struct page_mapper {
    int (*map)(virt_addr_t, phys_addr_t, uint32_t);
    int (*unmap)(virt_addr_t);
} page_mapper_t;

typedef struct frame_allocator {
    page_frame_t* (*alloc)(void);
    void (*free)(page_frame_t*);
} frame_allocator_t;
```