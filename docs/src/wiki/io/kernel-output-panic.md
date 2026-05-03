# Kernel Output and Panic Flow

This page describes how JANUS routes text output and fatal errors from kernel code to hardware devices.

## Why a Shared Output Path Exists

Early in boot, only serial output may be available. Later, framebuffer/TTY output may come online. Kernel code should not care which backend is active at a given moment.

JANUS solves this with a core service (`kio`) that sits between callers and concrete drivers.

## Data Flow

```text
caller (subsys/lib/kmain)
    -> kprintf / kpanic (kio)
    -> registered putc callback
    -> serial/tty drivers
```

`kio` provides a single API surface:

- `kprintf()`
- `vkprintf()`
- `kpanic()`

The callback is installed by console initialization in `kmain`.

## Layering

`kio` is in `kernel/core/` and depends on:

- `fmt` (string formatting)
- `janus_asm` (halt primitives)

It does not depend on driver subsystems directly. This preserves subsystem isolation while keeping output globally available.

## Panic Behavior

`kpanic()` prints:

1. panic banner
2. file and line
3. formatted reason

After printing, it halts forever via architecture-specific backend files selected by CMake.

This keeps architecture selection in build configuration and keeps raw instruction wrappers centralized in the asm layer.

## Related Pages

- [Core kio Reference](../../project/subsystems/core/kio.md)
- [Serial Ports](serial-ports.md)
- [Memory-Mapped I/O](mmio.md)
