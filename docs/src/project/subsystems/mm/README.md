# Memory Management

The `mm` subsystem owns physical memory allocation and memory-management lifecycle.

## Components

| Component | Description |
|---|---|
| [Physical Memory Manager](pmm.md) | Bitmap-backed 4 KiB frame allocator (`mm_pmm_*`) |

## Responsibilities

- Build the allocatable frame set from the boot memory map.
- Reserve low memory and the kernel image from allocation.
- Serve single-page frame allocation and free.
- Provide PMM statistics to the rest of the kernel.

`mm` currently focuses on PMM. Virtual memory management and higher-level allocators are planned on top of this foundation.
