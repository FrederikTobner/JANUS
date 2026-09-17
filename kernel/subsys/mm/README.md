# mm — Memory Management Subsystem

The `mm` subsystem owns physical memory allocation and memory-management lifecycle.

## Responsibilities

- **Physical memory manager (PMM)**
- **Virtual memory manager (VMM)**
- **TLB management**

## Verification

Heap allocator smoke testing remains available via
`-DJANUS_TEST_KMALLOC=ON`, but the allocator implementation now lives in the
core-layer `kmalloc` module. See
[kernel/core/kmalloc/README.md](../../core/kmalloc/README.md) for details.
