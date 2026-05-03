# Physical Memory Manager (PMM)

The PMM tracks allocatable physical frames with a bitmap and provides three main APIs:

- `mm_pmm_init()`
- `mm_pmm_alloc_page()`
- `mm_pmm_free_page()`

It also exposes `mm_pmm_get_stats()` for observability.

## Current Model

- Coverage: 4 GiB physical address space
- Frame size: 4 KiB
- Bitmap location: static `.bss` storage
- Bit meaning: 1 = free, 0 = used

## Initialization

Initialization is two-pass:

1. Mark usable/reclaimable regions from boot memmap as free.
2. Mark kernel image frames as used.

The allocator always keeps low memory below 1 MiB reserved.

## Allocation and Failure Semantics

`mm_pmm_alloc_page()` scans from `first_free_hint` and returns a physical page address on success.

On out-of-memory it returns `0` (reserved physical address, never a valid allocatable frame in this PMM model).

Programming errors (uninitialized PMM, invalid frees, double-free) are treated as fatal and handled with `kpanic()`.

## Safety Checks

The PMM enforces:

- initialization checks on alloc/free/stats
- free-address alignment checks
- range checks against tracked bitmap coverage
- double-free detection

These checks intentionally favor fail-fast behavior over silent corruption.
