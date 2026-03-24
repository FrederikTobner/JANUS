# Boot Subsystem

The boot subsystem bridges the gap between firmware-provided boot data and the kernel's internal representation. It defines a single, protocol-agnostic data structure — `boot_context_t` — and provides a uniform `boot_init()` entry point that each boot protocol implements independently.

## Design

The subsystem itself is an INTERFACE library: it ships headers only and compiles no code of its own. The actual initialisation logic lives in protocol-specific static libraries (`boot_limine`, `boot_multiboot2`) under `protocol/`. At link time, exactly one of these libraries is included, determined by the active boot protocol configuration. The linker resolves the `boot_init` symbol to whichever protocol was selected.

```text
boot (INTERFACE)                ← headers only: include/boot/context.h
  ├── boot_limine (STATIC)          ← protocol/limine/limine_boot.c
  └── boot_multiboot2 (STATIC)      ← protocol/multiboot2/multiboot2_boot.c
```

[!margin]
For a general explanation of how bootloaders and boot protocols work, see [Bootloaders](../../concepts/boot/bootloaders.md) in the Concepts section.
[/!margin]

## Data Flow

`kernel_main` allocates a `kernel_descriptor_t` on the stack, which embeds a `boot_context_t`. It then passes a pointer to the boot context into `boot_init()`. The protocol implementation reads firmware-provided data — Limine request responses or Multiboot2 tags — and populates every field of the context unconditionally.

After `boot_init()` returns, the boot context is fully populated and the rest of the init sequence can read framebuffer dimensions, the physical memory map, and other boot-time information through its fields.

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
