# MMU

The Memory Management Unit (MMU) is a hardware component that translates virtual addresses used by software into physical addresses used by the memory hardware. It enables features like virtual memory, memory protection, and address space isolation.

## TLB

The Translation Lookaside Buffer (TLB) is a cache that stores recent virtual-to-physical address translations.
It is part of the MMU and helps speed up address translation by avoiding the need to walk the page tables for every memory access.
When a virtual address is accessed, the MMU first checks the TLB for a cached translation. If it finds one, it uses that translation directly. 
If not, it performs a page table walk to find the translation and then caches it in the TLB for future accesses.
