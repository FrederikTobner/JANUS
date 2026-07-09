# Core Services

## kio

`kio` is the kernel's text output and fatal-error surface.
It decouples the formatted-output mechanism from the hardware that delivers it by accepting a single `putc` callback, registered once by `kmain` after the console backends are initialised.
All subsequent calls to `kprintf` or `kpanic` from any module flow through that single registered callback.

The panic surface is intentionally minimal.
The `kpanic(msg, ...)` macro injects `__FILE__` and `__LINE__` at the call site and forwards to `kpanic_impl`, which prints a structured banner and halts the CPU via `asm_cpu_halt_forever()`.
There is currently no support for stack unwinding and no attempt at recovery.
