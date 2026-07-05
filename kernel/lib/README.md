# lib — Kernel Libraries

Freestanding utility libraries shared across subsystems. Libraries may depend
on `janus_asm`, but never on other libraries, any core services or any subsystems.

## Libraries

### gfx

Stateless pixel primitives operating on a `gfx_surface_t` (plain public struct, caller-owned).
Provides pixel write, filled-rectangle fill, mono-bitmap blit, and scanline-copy scroll.
Fonts, text layout, and console semantics are out of scope.

### fmt

Formatted output to an arbitrary sink.
Provides `printf`-style formatting without depending on `libc`.
Consumers first supply a `fmt_putc_fn` callback, which is called for each character of the formatted output.

### page\_tables *(aarch64 only)*

MMU library for creating and managing 4-level page tables on AArch64.
Maps physical addresses into the kernel virtual address space.
