# Boot Subsystem

The boot subsystem bridges the gap between firmware-provided boot data and the kernel's internal representation. It defines a single, protocol-agnostic data structure — `boot_context_t` — and provides a uniform `boot_init()` entry point that each boot protocol implements independently.

## Design

The subsystem itself is an INTERFACE library: it ships headers only and compiles no code of its own. The actual initialisation logic lives in protocol-specific static libraries (`boot_limine`, `boot_multiboot2`) under `protocol/`. At link time, exactly one of these libraries is included, determined by the active boot protocol configuration. The linker resolves the `boot_init` symbol to whichever protocol was selected.

```text
boot (INTERFACE)                ← headers only: include/boot/context.h
  ├── boot_limine (STATIC)          ← protocol/limine/limine_boot.c
  └── boot_multiboot2 (STATIC)      ← protocol/multiboot2/multiboot2_boot.c
```

[!side]
For a general explanation of how bootloaders and boot protocols work, see [Bootloaders](../../../wiki/boot/bootloaders.md) in the Wiki.
[/!side]

## Components

| Component | Description |
|-----------|-------------|
| [Boot Context](context.md) | The `boot_context_t` structure and data flow |
| [Limine Protocol](limine.md) | Limine request/response implementation |
| [Multiboot2 Protocol](multiboot2.md) | Multiboot2 tag parsing implementation |

## Supported Protocols

| Protocol   | Architectures    | Notes |
|------------|-----------------|-------|
| Limine     | x86_64, aarch64 | Assembly entry calls `kernel_main` directly. |
| Multiboot2 | x86_64          | Assembly stashes the magic/info pointer before calling `kernel_main`. |

## Adding a New Protocol

1. Create `protocol/<name>/` with a CMakeLists.txt that builds a static library linking `PUBLIC boot`.
2. Implement `boot_init(boot_context_t *ctx)` — populate all fields unconditionally.
3. Add the assembly entry point(s) under `_start/<arch>/<name>/`.
4. Wire the protocol into `cmake/boot/` so it is selectable via `JANUS_BOOT_PROTOCOLS`.
