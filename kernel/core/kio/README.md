# kio — Kernel I/O Core Module

`kio` provides a shared text output and panic API for the kernel.

## Public API

- `kio_register_putc(kio_putc_fn fn)`
- `kprintf(char const *fmt, ...)`
- `vkprintf(char const *fmt, va_list ap)`
- `kpanic(fmt, ...)`

Header: `include/kio/output.h`

## Design

`kio` does not depend on hardware drivers directly. It writes through a registered callback (`kio_putc_fn`) provided by `kmain` output sink initialization (`output_sink_init` / `output_sink_init_early`).

Formatting is delegated to `lib/fmt`.

## Panic Halt Backend

Architecture-specific halt code lives in:

- `arch/x86_64/halt.c`
- `arch/aarch64/halt.c`

These backends use the asm layer wrappers (`<asm/cpu.h>`), keeping raw inline assembly out of core.

## Build Integration

`CMakeLists.txt` appends the correct architecture halt source based on `JANUS_TARGET_ARCH` and links:

- `fmt`
- `janus_asm`
