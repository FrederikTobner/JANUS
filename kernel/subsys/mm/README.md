# mm — Memory Management Subsystem

> **Status: placeholder** — no sources yet.

Will own physical memory, virtual memory, and page table lifecycle once
implemented.

## Planned Responsibilities

- **Physical memory manager (PMM)** — page frame allocator (bitmap or buddy)
- **Virtual memory manager (VMM)** — page mapping/unmapping via `lib/page_tables`
- **TLB management** — via `asm/tlb.h` and `asm/barriers.h`

## Relationship with `lib/page_tables`

`lib/page_tables` is a stateless algorithm library for manipulating AArch64
page table data structures. The `mm` subsystem will own the allocator that
backing memory comes from and the lifecycle of each address space. `mm` calls
into `page_tables`; `page_tables` does not call into `mm`.
