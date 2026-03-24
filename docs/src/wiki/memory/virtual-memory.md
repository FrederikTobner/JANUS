# Virtual Memory

Virtual memory is an abstraction that gives each program — and the kernel itself — the illusion of a private, contiguous address space, regardless of how physical memory is actually laid out. The hardware's memory management unit (MMU) translates every virtual address the CPU issues into a physical address before it reaches the memory bus, and this translation is governed by a set of data structures called page tables.

## Why Virtual Memory Exists

Without virtual memory, every program would need to know exactly where in physical RAM it resides, and no two programs could use the same address. Relocation would require patching every address in the binary. Memory protection would be impossible — any program could read or write any byte in the system.

Virtual memory solves all of these problems. Each address space has its own mapping from virtual to physical addresses, so multiple programs can use the same virtual addresses without conflict. The MMU enforces permissions on every access — read, write, execute — and generates a fault if a program attempts something its page table entries do not permit. The kernel can map physical memory wherever it likes in the virtual address space, placing the kernel in the upper half and user programs in the lower half, with a clear boundary between them.

## Paging

The translation mechanism is called paging. The virtual and physical address spaces are divided into fixed-size pages (typically 4 KiB). A page table maps each virtual page to a physical frame, along with permission bits and status flags. When the CPU accesses a virtual address, the MMU walks the page table hierarchy to find the corresponding physical address.

On x86_64, the page table is a four-level radix tree (PML4 → PDPT → PD → PT), where each level is a 4 KiB table of 512 entries. A virtual address is split into four 9-bit indices (one per level) plus a 12-bit page offset. Each entry contains the physical address of the next-level table (or the actual frame, at the final level) along with flags for present, writable, user-accessible, and execute-disable.

On aarch64, the structure is analogous — a four-level translation table system with 48-bit virtual addresses — though the terminology differs (translation table entries rather than page table entries, exception levels rather than privilege rings).

## The Translation Lookaside Buffer

Walking four levels of page tables for every memory access would be prohibitively slow. The CPU maintains a cache of recent translations called the TLB (Translation Lookaside Buffer). If the virtual address is found in the TLB, the physical address is returned immediately without a page table walk. When the kernel modifies the page tables, it must invalidate the affected TLB entries — failing to do so leaves stale translations in the cache and causes subtle, difficult-to-diagnose corruption.

## Higher-Half Kernels

A common convention in OS design is to map the kernel into the upper portion of the virtual address space (the "higher half") and reserve the lower portion for user-space programs. On x86_64 with 48-bit canonical addresses, the kernel typically occupies addresses starting at `0xFFFF800000000000` or higher. This arrangement allows user-space page tables to differ per process while the kernel mapping remains the same across all address spaces.

[!margin]
See [Physical Memory](physical-memory.md) for how the underlying physical frames are managed.
[/!margin]
