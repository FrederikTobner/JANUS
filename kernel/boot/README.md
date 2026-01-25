# TinyOS Boot Module

Boot-protocol glue and the very earliest execution code that runs before the “normal” kernel world exists.

This module is structured as `(arch, boot protocol)` implementations selected at configure time via:

- `TINYOS_TARGET_ARCH`
- `TINYOS_BOOT_PROTOCOL`

## Responsibilities

- Provide the protocol-specific entry stub (typically in assembly)
- Do the minimum early CPU setup required to reach C code
- Construct a minimal, stable handoff structure and call into the kernel entry point
- Perform protocol-specific “sanity checks” (so `kernel/core` stays protocol-neutral)

## Public interface

Headers live under `include/boot/`:

- `handoff.h` — `struct boot_handoff`, the minimal ABI passed to `kernel_main()`
- `protocols.h` — `boot_protocol_t` and protocol IDs
- `verify.h` — `boot_verify_handoff()` (protocol-based validation)
- `multiboot2.h` — Multiboot2 structures/constants (used by the Multiboot2 implementation)

## Current implementation

Only one `(arch, protocol)` pair is implemented right now:

- `x86_64/multiboot2/`
  - `multiboot2.asm` — Multiboot2 header
  - `boot.asm` — early entry / long-mode transition / calls `kernel_main(struct boot_handoff const *)`
  - `verify.c` — Multiboot2-based implementation of `boot_verify_handoff()`

## Directory layout

```text
boot/
├── CMakeLists.txt                  # selects boot/<arch>/<protocol>/
├── include/
│   └── boot/
│       ├── handoff.h
│       ├── multiboot2.h
│       ├── protocols.h
│       └── verify.h
└── x86_64/
    └── multiboot2/
        ├── CMakeLists.txt
        ├── boot.asm
        ├── multiboot2.asm
        └── verify.c
```

Note: `boot/boot.asm` and `boot/multiboot2.asm` may exist as legacy files, but the build is wired to use the per-arch/per-protocol implementation folders.

## Build integration

`boot/CMakeLists.txt` is a selector that adds `boot/${TINYOS_TARGET_ARCH}/${TINYOS_BOOT_PROTOCOL}`.
If the folder (or its `CMakeLists.txt`) does not exist, configuration fails fast with a helpful error.

## Adding a new boot protocol

Minimal convention:

1. Create `boot/<arch>/<protocol>/` with a `CMakeLists.txt` that builds a `boot` object library.
2. Provide a protocol-specific entry path that constructs a `struct boot_handoff`.
3. Implement `boot_verify_handoff()` for that protocol (can start as a minimal check, then grow later).
