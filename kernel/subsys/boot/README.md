# JANUS Boot Subsystem

Protocol-agnostic boot context definitions and protocol-specific initialization
implementations.

## Current Implementations

| Protocol   | Supported Architectures | Entry point setup                                              |
|------------|-------------------------|----------------------------------------------------------------|
| Limine     | x86_64, aarch64         | Assembly calls `kernel_main` directly                          |
| Multiboot2 | x86_64                  | Assembly calls `multiboot2_stash_bootinfo`, then `kernel_main` |

## Responsibilities

- Define the `boot_context_t` structure (public, not opaque) that captures all boot-time information the kernel needs
- Provide a common `boot_init(boot_context_t *)` entry point, which is implemented by each protocol
- Parse protocol-specific data (Limine responses, Multiboot2 tags) into the uniform boot context

## Architecture

The boot subsystem is an **INTERFACE library**.
The actual initialization code lives in the protocol libraries under `protocol/`, which are plain `add_library(STATIC)` targets that link against `boot` to inherit its include paths.

`kernel_main` calls `boot_init(&kd.boot)`.
The linker resolves this to the symbol in the proper protocol library, that was linked

## Adding a New Boot Protocol

1. Create `protocol/<name>/` with a `CMakeLists.txt` that builds a static library (e.g., `boot_<name>`) and links `PUBLIC boot`
2. Implement `int boot_init(boot_context_t * ctx)` — populate all fields of the    context unconditionally (no reliance on zero-initialization)
3. Add assembly entry point(s) under `_start/<arch>/<name>/`
4. Wire the protocol into `cmake/boot/` via the `JANUS_BOOT_PROTOCOLS` variable in cmake, so it becomes a selectable build target
