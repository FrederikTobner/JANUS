# lib — Kernel Libraries

Freestanding utility libraries shared across subsystems.
Libraries may depend on `janus_asm`, but never on other libraries, any core services or any subsystems.

## Libraries

### fmt

Formatted output to an arbitrary sink.

More information about the subsystem can be found in its [README](fmt/README.md)

### gfx

Provides stateless pixel primitives operating on a `gfx_surface_t`. like pixel write, filled-rectangle fill, mono-bitmap blit, and scanline-copy scroll.

More information about the subsystem can be found in its [README](gfx/README.md)

### mem

Freestanding byte-level memory primitives.

More information about the subsystem can be found in its [README](mem/README.md)

### page\_tables

MMU library for creating and managing 4-level page tables.
Maps physical addresses into the kernel virtual address space.

More information about the subsystem can be found in its [README](page_tables/README.md)
