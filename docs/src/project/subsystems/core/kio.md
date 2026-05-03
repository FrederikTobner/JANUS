# kio

`kio` is the kernel I/O core module. It centralizes formatted output and panic handling behind a small API:

- `kio_register_putc()`
- `kprintf()`
- `vkprintf()`
- `kpanic()`

## Design

`kio` does not talk to hardware directly. Instead, it accepts a character output callback (`kio_putc_fn`) registered by `kmain` once the console path is initialized.

This gives a single output surface for every layer while preserving dependency isolation:

- `kio` depends on `fmt` for formatting
- `kio` depends on `janus_asm` for halt primitives
- `kio` does not depend on `drivers` or any other subsystem

## Panic Path

`kpanic()` prints a banner and source location, then halts forever via architecture-specific backend files selected by CMake:

- `kernel/core/kio/arch/x86_64/halt.c`
- `kernel/core/kio/arch/aarch64/halt.c`

This keeps architecture selection in CMake and keeps inline assembly confined to the asm layer wrappers (`asm/cpu.h`).

## Typical Flow

1. `kmain` calls `console_init_early()`.
2. Console setup registers `console_putc` through `kio_register_putc()`.
3. Subsystems call `kprintf()` or `kpanic()` without depending on driver internals.
