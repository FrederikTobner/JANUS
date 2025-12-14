# TinyOS Documentation Style Guide

## Overview

**Document why, not what. If your comment just repeats what the code says, delete it.**

Documentation in TinyOS has a purpose:

- **Explain complex logic** - Algorithms and design decisions that aren't obvious
- **Define APIs** - Interfaces and contracts
- **Educational value** - Help people learn, not just copy-paste
- **Enable debugging** - Future you needs to understand this code at 3AM

## Comment Principles

### Write Comments That Explain "Why", Not "What"

**If your comment just describes what the code does, you're wasting everyone's time.**

```c
// USELESS: Anyone can read C
// Increment the counter
counter++;

// USEFUL: Explains the reason
// Track page faults for memory pressure detection
counter++;
```

**Can a competent C programmer figure out what the code does by reading it? Then don't comment it. Comment why it exists, how it can be used, but not what it does.**

### Document Intent and Invariants

```c
// GOOD: Documents preconditions and invariants
/**
 * Allocate a physical page frame
 * 
 * Preconditions:
 * - Physical memory manager must be initialized
 * - Called with interrupts enabled (may need to wait)
 * 
 * Postconditions:
 * - Returned page is zeroed
 * - Returned page is not shared with any other allocation
 * 
 * @return Physical address of allocated page, or 0 if out of memory
 */
uint64_t pmm_alloc_page(void);
```

### Explain Non-Obvious Design Decisions

```c
// GOOD: Explains subtle implementation choice
/*
 * Use identity mapping for first 4MB to avoid page fault during
 * early boot. This region will be unmapped after we set up the
 * high half kernel mapping at 0xFFFFFFFF80000000.
 */
page_table[0] = 0x00000000 | PAGE_PRESENT | PAGE_WRITE;
```

## Function Documentation

### Function Header Format

Use Doxygen-style comments:

```c
/**
 * Brief one-line description of function
 * 
 * Optional detailed description explaining the function's purpose,
 * algorithm, or important implementation notes.
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * @error Error conditions and return values
 */
int function_name(type1 param1, type2 param2);
```

### Complete Example

```c
/**
 * Map a virtual address to a physical address
 * 
 * Creates or updates a page table entry to establish the mapping.
 * If intermediate page tables don't exist, they are allocated.
 * 
 * @param virt_addr Virtual address to map (must be page-aligned)
 * @param phys_addr Physical address to map to (must be page-aligned)
 * @param flags Page flags (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)
 * @return 0 on success, -ENOMEM if page table allocation fails,
 *         -EINVAL if addresses are not page-aligned
 */
int vm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint32_t flags);
```

### Parameter Documentation

```c
/**
 * Copy memory between buffers
 * 
 * @param dest Destination buffer (must not overlap with src)
 * @param src Source buffer (must not overlap with dest)
 * @param count Number of bytes to copy (dest must have room)
 * @return Pointer to dest
 */
void * memcpy(void * dest, void const * src, size_t count);
```

### Return Value Documentation

```c
/**
 * Allocate memory from kernel heap
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory on success, NULL if:
 *         - size is 0
 *         - Out of memory
 *         - size exceeds maximum allocation size
 */
void * kmalloc(size_t size);
```

## Inline Comments

### When to Use Inline Comments

```c
void handle_page_fault(uint64_t fault_addr) {
    // Save fault address before it's overwritten
    uint64_t addr = fault_addr;
    
    // Check if fault is in user space
    if (addr < USER_SPACE_END) {
        // User space page fault - check if it's a valid mapping
        if (!is_valid_user_address(addr)) {
            terminate_process(SIGSEGV);
            return;
        }
    }
    
    // Allocate new page for demand paging
    uint64_t page = pmm_alloc_page();
    if (page == 0) {
        // OOM - kill process
        terminate_process(SIGKILL);
        return;
    }
    
    vm_map_page(addr & PAGE_MASK, page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
}
```

### Section Markers

Use comments to divide long functions into logical sections:

```c
void complex_initialization(void) {
    /*
     * Phase 1: Initialize basic data structures
     */
    init_hashtable(&symbol_table, 256);
    init_allocator(&temp_allocator, 4096);
    
    /*
     * Phase 2: Parse configuration
     */
    config_t * config = parse_config_file("/etc/tinyos.conf");
    if (config == NULL) {
        // Fall back to defaults
        config = get_default_config();
    }
    
    /*
     * Phase 3: Apply configuration
     */
    apply_memory_settings(config->memory);
    apply_device_settings(config->devices);
}
```

## File Headers

### Source File Header

```c
/**
 * Physical Memory Manager
 * 
 * Manages allocation and deallocation of physical page frames using a
 * bitmap allocator. Tracks memory zones (DMA, Normal, High) separately.
 * 
 * Implementation uses a stack-based allocator for recently freed pages
 * to improve cache locality.
 * 
 * Author: [Name]
 * Date: [Date]
 */

#include <kernel/pmm.h>
#include <kernel/memory.h>
#include <lib/fio/fio.h>
```

### Header File Documentation

```c
/**
 * Character Buffer Interface
 * 
 * Provides a resizable character buffer with automatic memory management.
 * Buffers grow automatically when capacity is exceeded.
 * 
 * Example usage:
 *   char_buffer_t buf;
 *   char_buffer_init(&buf, 256);
 *   char_buffer_append(&buf, "Hello, ");
 *   char_buffer_append(&buf, "World!");
 *   kprintf("%s\n", buf.data);
 *   char_buffer_free(&buf);
 */

#ifndef TINYOS_LIB_CHAR_BUFFER_H
#define TINYOS_LIB_CHAR_BUFFER_H

#include <stdint.h>
#include <stddef.h>
```

## Structure and Type Documentation

### Structure Documentation

```c
/**
 * Process control block
 * 
 * Contains all state necessary to schedule and execute a process.
 * One PCB exists for each process in the system.
 */
typedef struct process {
    uint64_t pid;              // Process identifier (unique)
    uint64_t parent_pid;       // Parent process identifier
    
    /**
     * Virtual address space
     * Points to the root page table (CR3 value)
     */
    uint64_t page_directory_phys;
    
    /**
     * CPU state saved during context switch
     * Restored when process is scheduled
     */
    cpu_state_t cpu_state;
    
    /**
     * Process state
     * One of: RUNNING, READY, BLOCKED, ZOMBIE
     */
    process_state_t state;
    
    /**
     * Scheduling priority
     * Lower values = higher priority (0 is highest)
     */
    uint8_t priority;
} process_t;
```

### Enumeration Documentation

```c
/**
 * Page allocation flags
 */
typedef enum {
    PAGE_ZONE_DMA    = 0x01,  // Allocate from DMA-capable zone (< 16MB)
    PAGE_ZONE_NORMAL = 0x02,  // Allocate from normal zone
    PAGE_ZERO        = 0x10,  // Zero page after allocation
    PAGE_NO_WAIT     = 0x20   // Return immediately if no pages available
} page_alloc_flags_t;
```

### Bit Field Documentation

```c
/**
 * Page table entry (x86-64)
 * 
 * Hardware-defined format:
 * Bits 0-11:   Flags
 * Bits 12-51:  Physical address (shifted right by 12)
 * Bits 52-63:  Available for software use
 */
typedef uint64_t pte_t;

// Flag definitions
#define PTE_PRESENT    (1ULL << 0)   // Page is present in memory
#define PTE_WRITE      (1ULL << 1)   // Page is writable
#define PTE_USER       (1ULL << 2)   // Page is accessible from user mode
#define PTE_WRITETHROUGH (1ULL << 3) // Write-through caching
#define PTE_CACHE_DISABLE (1ULL << 4) // Disable caching
#define PTE_ACCESSED   (1ULL << 5)   // Page has been accessed (set by hardware)
#define PTE_DIRTY      (1ULL << 6)   // Page has been written to (set by hardware)
#define PTE_PAT        (1ULL << 7)   // Page Attribute Table
#define PTE_GLOBAL     (1ULL << 8)   // Global page (not flushed on CR3 write)
#define PTE_NX         (1ULL << 63)  // No execute
```

## Algorithm Documentation

### Algorithm Explanation

```c
/**
 * Quicksort implementation
 * 
 * Uses Lomuto partition scheme with median-of-three pivot selection
 * to avoid worst-case O(n²) behavior on already-sorted data.
 * 
 * Time complexity:
 * - Average case: O(n log n)
 * - Worst case:   O(n²) (rare with median-of-three)
 * - Best case:    O(n log n)
 * 
 * Space complexity: O(log n) for recursion stack
 * 
 * Not stable (equal elements may be reordered).
 */
void quicksort(int * array, size_t length);
```

### Step-by-Step Algorithm

```c
/**
 * Bootstrap the kernel memory allocator
 * 
 * Algorithm:
 * 1. Get memory map from bootloader
 * 2. Find largest contiguous free region
 * 3. Reserve region for page frame bitmap
 * 4. Initialize bitmap (mark all pages as used)
 * 5. Walk memory map and mark free pages
 * 6. Reserve kernel image and initial data structures
 * 
 * After this function, pmm_alloc_page() can be called.
 */
void pmm_init(multiboot_info_t * mboot_info);
```

## Error Handling Documentation

### Document Error Conditions

```c
/**
 * Open a file
 * 
 * @param path File path (must be absolute)
 * @param flags Open flags (O_RDONLY, O_WRONLY, O_RDWR)
 * @return File descriptor on success, or negative error code:
 *         -ENOENT: File does not exist
 *         -EACCES: Permission denied
 *         -EISDIR: Path is a directory
 *         -ENFILE: System file table full
 *         -EINVAL: Invalid flags
 */
int vfs_open(char const * path, int flags);
```

## Hardware-Specific Documentation

### Register Definitions

```c
/**
 * PIC (8259 Programmable Interrupt Controller) ports
 */
#define PIC1_COMMAND   0x20  // PIC1 command port
#define PIC1_DATA      0x21  // PIC1 data port
#define PIC2_COMMAND   0xA0  // PIC2 command port
#define PIC2_DATA      0xA1  // PIC2 data port

/**
 * PIC Initialization Command Words (ICW)
 */
#define ICW1_ICW4      0x01  // ICW4 needed
#define ICW1_SINGLE    0x02  // Single (cascade) mode
#define ICW1_INTERVAL4 0x04  // Call address interval 4 (8)
#define ICW1_LEVEL     0x08  // Level triggered mode
#define ICW1_INIT      0x10  // Initialization required

/**
 * Initialize PIC to remap IRQs
 * 
 * Default PC configuration maps IRQ 0-7 to interrupts 8-15, which
 * conflicts with CPU exceptions. This function remaps:
 * - IRQ 0-7  → interrupts 32-39
 * - IRQ 8-15 → interrupts 40-47
 */
void pic_remap(void);
```

### Hardware Quirks

```c
/**
 * Enable A20 gate using keyboard controller
 * 
 * The A20 gate is a legacy hardware "feature" that gates the 21st
 * address line. If disabled, addresses wrap around at 1MB.
 * 
 * WARNING: Some systems require a delay after the keyboard controller
 * write. We use io_wait() to ensure compatibility.
 */
void enable_a20_keyboard(void) {
    // Disable keyboard
    outb(0x64, 0xAD);
    
    // Read output port
    outb(0x64, 0xD0);
    io_wait();  // Critical delay for some hardware
    uint8_t port = inb(0x60);
    
    // Write output port with A20 enabled (bit 1)
    outb(0x64, 0xD1);
    io_wait();  // Critical delay
    outb(0x60, port | 0x02);
    
    // Enable keyboard
    outb(0x64, 0xAE);
}
```

## Assembly Documentation

### Assembly Block Comments

```c
/**
 * Load GDT and perform far jump to reload code segment
 * 
 * @param gdt_ptr Pointer to GDT descriptor (16-bit limit + 64-bit base)
 * @param code_segment Code segment selector (usually 0x08)
 */
static inline void load_gdt(void const * gdt_ptr, uint16_t code_segment) {
    __asm__ volatile(
        "lgdt (%0)\n"           // Load GDT
        "push %1\n"             // Push code segment
        "lea 1f(%%rip), %%rax\n" // Load address of label 1
        "push %%rax\n"          // Push return address
        "lretq\n"               // Far return (loads CS)
        "1:\n"                  // Label 1: execution continues here
        :
        : "r"(gdt_ptr), "r"((uint64_t) code_segment)
        : "rax", "memory"
    );
}
```

## TODO and FIXME Comments

### Standard Markers

```c
// TODO: Implement proper scheduling algorithm
void schedule_next_process(void) {
    // Currently just round-robin
}

// FIXME: Race condition when interrupts are enabled
void unsafe_function(void) {
    // Need to add spinlock protection
}

// HACK: Workaround for hardware bug in early x86-64 CPUs
void apply_errata_workaround(void) {
    // See Intel errata AAK49 for details
}

// NOTE: This assumes bootloader has identity-mapped first 4MB
void * early_boot_ptr = (void *) 0x100000;
```

### Detailed TODOs

```c
/*
 * TODO: Optimize page allocation
 * 
 * Current implementation uses a simple bitmap, which requires
 * scanning for free pages. Consider:
 * 
 * 1. Buddy allocator for power-of-2 sized allocations
 * 2. Free list with LIFO for cache locality
 * 3. Per-CPU page caches to reduce contention
 * 
 * Benchmark with realistic workload before changing.
 */
```

## Technical Writing Style

### Be Precise and Concrete

```c
// BAD: Vague
// Set up the memory system
void init_memory(void);

// GOOD: Specific
/**
 * Initialize physical memory manager
 * 
 * Parses the multiboot memory map, builds a bitmap of available
 * page frames, and reserves memory for the kernel image.
 */
void pmm_init(multiboot_info_t * mboot_info);
```

### Use Active Voice

```c
// BAD: Passive voice
// The page is freed by this function
void free_page(uint64_t page);

// GOOD: Active voice
/**
 * Free a physical page frame
 */
void free_page(uint64_t page);
```

### Avoid Redundancy

```c
// BAD: Redundant
/**
 * Calculate the size
 * 
 * This function calculates the size of the buffer.
 */
size_t calculate_size(buffer_t * buf);

// GOOD: Concise
/**
 * Calculate buffer size including overhead
 */
size_t calculate_size(buffer_t * buf);
```

## Examples and Usage

### Provide Usage Examples

```c
/**
 * Hashtable for generic key-value storage
 * 
 * Example usage:
 * 
 *   hashtable_t table;
 *   hashtable_init(&table, 64);
 *   
 *   char * key = "example";
 *   void * value = some_data;
 *   hashtable_insert(&table, key, strlen(key), value);
 *   
 *   void * retrieved = hashtable_lookup(&table, key, strlen(key));
 *   if (retrieved) {
 *       // Use retrieved value
 *   }
 *   
 *   hashtable_free(&table);
 */
```

## Reference Documentation

### Link to Specifications

```c
/**
 * Initialize Interrupt Descriptor Table
 * 
 * Sets up the IDT with handlers for all 256 interrupt vectors.
 * See Intel SDM Vol. 3A, Section 6.10 for IDT format details.
 * 
 * Reference: https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html
 */
void idt_init(void);
```

### Cross-Reference Related Functions

```c
/**
 * Allocate virtual memory region
 * 
 * See also:
 * - vm_free() to deallocate the region
 * - vm_map_page() to map physical pages into the region
 * - pmm_alloc_page() to allocate backing physical memory
 */
void * vm_alloc(size_t size, uint32_t flags);
```

## Documentation Maintenance

### Keep Documentation Current

1. **Update comments when changing code**
   - Stale comments are worse than no comments

2. **Review documentation in code reviews**
   - Ensure comments match implementation

3. **Remove obsolete comments**
   - Delete comments for removed code
   - Update comments for refactored code

### Document Assumptions

```c
/**
 * Parse ELF binary
 * 
 * Assumptions:
 * - Input buffer contains valid ELF file
 * - ELF is for the same architecture as kernel (x86-64)
 * - Program headers fit within buffer
 * 
 * These assumptions are validated and will return error if violated.
 */
int elf_parse(void const * buffer, size_t size, elf_info_t * info);
```
