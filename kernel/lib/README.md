# lib — Kernel Libraries

Freestanding utility libraries shared across subsystems. Libraries may depend
on `janus_asm` and other libraries, but never on subsystems.

## Libraries

### display

Header-only library providing the shared `display_info_t` type and
`display_mode_t` enum. Lives in `lib/` so both the `boot` subsystem (which
populates it) and the `drivers` subsystem (which consumes it) can depend on it
without depending on each other.

- `include/display/display.h` — `display_info_t`, `display_mode_t`

### fmt

Formatted output to an arbitrary sink. Provides `printf`-style formatting
without depending on `libc`. Consumers supply a `fmt_putc_fn` callback;
the library handles all format logic.

- `include/fmt/output.h` — `fmt_to()`, `vfmt_to()`, `fmt_putc_fn`
- `src/output.c` — Implementation

### page\_tables *(aarch64 only)*

MMU library for creating and managing 4-level page tables on AArch64.
Maps physical addresses into the kernel virtual address space. Uses
`asm/barriers.h`, `asm/tlb.h`, and `asm/regs.h` for TLB maintenance and
system register access.

- `include/page_tables/mmu.h` — `mmu_init()`, `mmu_map_mmio()`
- `arch/aarch64/mmu.c` — Implementation
