# JANUS Boot Subsystem

Protocol-agnostic boot context definitions and protocol-specific initialization
implementationsii.

## Responsibilities

- Define the `boot_context_t` structure (public, not opaque) that captures all
  boot-time information the kernel needs
- Provide a common `boot_init(boot_context_t *)` entry point — each protocol
  implements this symbol, and exactly one is linked per build configuration
- Parse protocol-specific data (Limine responses, Multiboot2 tags) into the
  uniform boot context

## Public Interface

Headers live under `include/boot/`:

- `context.h` — `boot_context_t`, `boot_display_info_t`, `boot_protocol_t`,
  `boot_init()` declaration, and inline address-translation helpers

## Architecture

The boot subsystem is an **INTERFACE library** (no compiled sources of its own).
It provides headers only. The actual initialization code lives in the protocol
libraries under `protocol/`, which are plain `add_library(STATIC)` targets that
link against `boot` to inherit its include paths.

```text
boot (INTERFACE)          ← headers only: include/boot/context.h
  ├── boot_limine (STATIC)     ← protocol/limine/limine_boot.c
  └── boot_multiboot2 (STATIC) ← protocol/multiboot2/multiboot2_boot.c (x86_64 only)
```

`kernel_main` calls `boot_init(&kd.boot)` — the linker resolves this to
whichever protocol library was linked for the active boot protocol.

## Current Implementations

| Protocol   | Architectures       | Entry point setup                |
|------------|---------------------|----------------------------------|
| Limine     | x86_64, aarch64     | Assembly calls `kernel_main` directly |
| Multiboot2 | x86_64              | Assembly calls `multiboot2_stash_bootinfo`, then `kernel_main` |

### Limine (`protocol/limine/`)

- `limine_boot.c` — implements `boot_init()`: reads Limine request responses
  (HHDM, executable address, framebuffer) and populates the boot context
- `limine_protocol.h` — Limine protocol structures and request declarations

Limine request symbols are defined in `_start/common/limine_requests.c` (shared
across architectures).

### Multiboot2 (`protocol/multiboot2/`)

- `multiboot2_boot.c` — implements `boot_init()` and `multiboot2_stash_bootinfo()`:
  the stash function saves the magic/info pointer from CPU registers (called by
  assembly before `kernel_main`), then `boot_init()` validates and parses the
  Multiboot2 tag list
- `multiboot2_protocol.h` — Multiboot2 structures, tag constants, and tag
  iteration helpers

## Directory Layout

```text
boot/
├── CMakeLists.txt              # INTERFACE library + add_subdirectory for protocols
├── README.md
├── include/
│   └── boot/
│       └── context.h           # Public boot context API
└── protocol/
    ├── limine/
    │   ├── CMakeLists.txt
    │   ├── limine_boot.c       # boot_init() for Limine
    │   └── limine_protocol.h
    └── multiboot2/
        ├── CMakeLists.txt
        ├── multiboot2_boot.c   # boot_init() + multiboot2_stash_bootinfo()
        └── multiboot2_protocol.h
```

## Adding a New Boot Protocol

1. Create `protocol/<name>/` with a `CMakeLists.txt` that builds a static library
   (e.g., `boot_<name>`) and links `PUBLIC boot`
2. Implement `int boot_init(boot_context_t * ctx)` — populate all fields of the
   context unconditionally (no reliance on zero-initialization)
3. Add assembly entry point(s) under `_start/<arch>/<name>/`
4. Wire the protocol into `cmake/boot/` so it is selectable via
   `JANUS_BOOT_PROTOCOLS`
