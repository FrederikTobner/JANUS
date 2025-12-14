# mm - Memory Management Module

Kernel memory management subsystem.

## Purpose

Manages physical and virtual memory, page allocation, and kernel heap.

## Planned Contents

### Physical Memory Management
- Page frame allocation and freeing
- Memory zone management (DMA, Normal, High)
- Physical memory bitmap/buddy allocator

### Virtual Memory Management
- Page table management (PML4, PDPT, PD, PT)
- Virtual address space creation and destruction
- Page mapping and unmapping
- TLB management

### Kernel Heap
- `kmalloc()` / `kfree()` - Dynamic kernel memory allocation
- Slab allocator for fixed-size objects
- Memory pool management

## Key Components

```
mm/
├── page_frame.c      # Physical page allocator
├── vmm.c             # Virtual memory manager
├── kmalloc.c         # Kernel heap allocator
└── include/mm/
    ├── page_frame.h
    ├── vmm.h
    └── kmalloc.h
```

## Dependencies

- `types` - Type definitions
- `memory` - Memory operations
- `arch` - Architecture-specific page table operations

## Usage Example

```c
#include <mm/page_frame.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>

// Allocate physical page
page_frame_t* frame = mm_alloc_page_frame();

// Map virtual to physical
mm_map_page(vaddr, frame->phys_addr, MM_FLAG_WRITABLE);

// Kernel heap allocation
void* ptr = kmalloc(1024);
kfree(ptr);
```

## Status

⚠️ **Placeholder** - Implementation pending in future development phases.
