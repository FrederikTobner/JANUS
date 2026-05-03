# mm — Memory Management Subsystem

The `mm` subsystem owns physical memory allocation and memory-management lifecycle.

## Responsibilities

- **Physical memory manager (PMM)** — page frame allocator (bitmap or buddy)
- **Virtual memory manager (VMM)** — page mapping/unmapping via `lib/page_tables`
- **TLB management** — via `asm/tlb.h` and `asm/barriers.h`

## Current Status

- PMM is implemented in `src/pmm.c`.
- Allocator granularity is one 4 KiB frame.
- Frame ownership is tracked with a bitmap.
- API includes init, alloc, free, and stats.
- Safety checks include invalid free detection and double-free detection.

## Relationship with `lib/page_tables`

`lib/page_tables` is a stateless algorithm library for manipulating AArch64
page table data structures. The `mm` subsystem will own the allocator that
backing memory comes from and the lifecycle of each address space. `mm` calls
into `page_tables`; `page_tables` does not call into `mm`.
