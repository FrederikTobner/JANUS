# mm — Memory Management Subsystem

Kernel memory management subsystem.

## Purpose

Will manage physical and virtual memory, page allocation, and heap growth
policy. Currently a placeholder — no sources yet.

## Planned Contents

### Physical Memory Management

- Page frame allocation and freeing
- Memory zone management (DMA, Normal, High)
- Physical memory bitmap/buddy allocator

### Virtual Memory Management

- Page table management via `lib/page_tables`
- Virtual address space creation and destruction
- Page mapping and unmapping
- TLB management

### Heap Growth Backend

- Provides the page-backed heap growth callback for `lib/mem_alloc`
- `lib/mem_alloc` (the allocation API: `kalloc` / `kfree`) lives in `kernel/lib/`
  as a support library — this subsystem provides the backing store via
  `kalloc_set_backend()` when it is ready

## Relationship with `lib/mem_alloc`

```text
lib/mem_alloc/     ← Stable API: kalloc(), kfree() (support library)
subsys/mm/         ← Policy: PMM, VMM, heap growth (this subsystem)
```

`lib/mem_alloc` starts with a static bump allocator (no dependencies).
Once `mm` is initialized, it injects a page-backed backend via function
pointers — `lib/mem_alloc` never includes a subsystem header.

## Status

⚠️ **Placeholder** — Implementation pending. The early-boot allocator
(`kalloc` / `kfree`) is provided by `lib/mem_alloc` independently.
