# Memory

This section covers how an OS kernel discovers, tracks, and maps RAM — from the raw physical frames provided by the firmware memory map to the virtual address spaces that isolate the kernel from user programs.

**Prerequisites:** [Boot](../boot/boot.md). The memory map is delivered by the bootloader, so you need to understand what the bootloader provides before reasoning about how the kernel consumes it.

## Articles

Read in order.

1. **[Physical Memory](physical-memory.md)** — The firmware memory map, page frames as the unit of allocation, and the physical memory manager (PMM) that tracks free frames.
2. **[Virtual Memory](virtual-memory.md)** — Why virtual address spaces exist, how paging translates virtual to physical addresses, the page table hierarchy on x86_64 and aarch64, TLB management, and the higher-half kernel convention.
