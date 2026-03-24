# Physical Memory

Physical memory refers to the actual RAM chips installed in the machine, addressed by physical addresses that correspond directly to locations on the memory bus. From the perspective of an operating system kernel, physical memory is the raw resource that must be discovered, catalogued, and parcelled out — to the kernel's own data structures, to page tables, and eventually to user-space processes.

## The Memory Map

A machine's physical address space is not a single flat slab of usable RAM. It is a patchwork of regions: some contain usable memory, others are reserved by firmware, mapped to device registers (MMIO), or occupied by the ACPI tables. The firmware communicates this layout to the bootloader through a memory map — a list of regions, each tagged with a type indicating whether the region is usable, reserved, ACPI-reclaimable, or otherwise unavailable.

The kernel must consult this memory map before allocating any physical memory. Writing to a reserved or MMIO region will corrupt firmware state or interact with hardware in unpredictable ways. The memory map is typically provided by the bootloader as part of the boot information structure.

## Page Frames

Physical memory is managed in fixed-size units called page frames. On both x86_64 and aarch64, the standard page frame size is 4 KiB (4096 bytes). Larger frames (2 MiB, 1 GiB) exist for performance-sensitive mappings, but 4 KiB frames are the fundamental unit of physical memory allocation.

A physical memory allocator — the PMM — tracks which frames are free and which are in use. The simplest viable allocator is a bitmap: one bit per frame, where a set bit indicates the frame is allocated. More sophisticated designs include free lists and buddy allocators, which trade space for faster allocation and the ability to hand out contiguous runs of frames.

## Physical Addresses vs. Virtual Addresses

A physical address identifies a byte in the machine's physical address space. In a system with paging enabled, the CPU does not use physical addresses directly — it translates virtual addresses through the page tables to arrive at physical addresses. The kernel must maintain the ability to convert between the two, typically through a direct map (a region of virtual address space that is identity-mapped or offset-mapped to all of physical memory).

[!margin]
See [Virtual Memory](virtual-memory.md) for how paging and address translation work.
[/!margin]
