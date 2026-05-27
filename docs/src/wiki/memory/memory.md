# Memory

This section covers how an OS kernel discovers, tracks, and maps RAM — from the raw physical frames provided by the firmware memory map to the virtual address spaces that isolate the kernel from user programs.

**Prerequisites:** [Boot](../boot/boot.md). The memory map is delivered by the bootloader, so you need to understand what the bootloader provides before reasoning about how the kernel consumes it.

## Content 

1. **[Physical Memory](physical-memory.md)**
2. **[Virtual Memory](virtual-memory.md)** 
3. **[MMU](mmu.md)** 
