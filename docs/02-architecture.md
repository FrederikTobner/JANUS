# Architecture

## Layer Model

Dependencies flow strictly downward. A module may depend on any module in a lower
layer, never on the same layer or above. CMake enforces this via target
dependencies.

```
kernel/_start/       Entry layer    — creates kernel.elf, owns entry point and linker script
kernel/kmain/        Composition    — only module allowed to depend on subsystems
kernel/subsys/       Subsystems     — boot, drivers, mm (isolated from each other)
kernel/core/         Core services  — kio (output + panic); may use libs, not subsystems
kernel/lib/          Libraries      — fmt, display, page_tables; no cross-lib deps
kernel/asm/          ASM layer      — janus_asm INTERFACE; owns raw instruction wrappers
kernel/include/      Global headers — types.h, attributes.h, config.h
kernel/contracts/    Contracts      — shared types crossing subsystem boundaries
```

**`_start`** links `kmain` and all subsystem libraries into `kernel.elf`. It
contains the assembly entry point, the boot protocol header, and the linker script.

**`kmain`** is the sole module that may depend on subsystems. All inter-subsystem
coordination flows through it — the star topology means subsystems never call each
other.

**Subsystems** are strictly isolated. `boot` cannot call `drivers`, `drivers`
cannot call `mm`. If two subsystems need to share information, the data flows
through `kmain` via an explicit parameter (typically in `kernel_descriptor_t`).

**Core services** sit between `lib/` and `subsys/`. They provide cross-cutting
functionality (formatted output, panic) that multiple subsystems need. Core modules
may depend on libraries but not on subsystems.

**Libraries** are freestanding utilities with no knowledge of the subsystems above
them. No library may depend on another library.

**The ASM layer** is the only layer that owns raw instruction wrappers. Higher
layers consume these through `janus_asm` instead of embedding inline assembly.

**Contracts** are type definitions shared across exactly N named subsystem
boundaries. They live in `kernel/contracts/<name>/include/contracts/<name>.h` and
are not in any default include search path. CMake enforces an allowlist of
authorised consumers via `janus_add_contract`; an unauthorised direct link fails
configure.

## Directory Layout

```
kernel/
├── include/          Global headers (janus/types.h, janus/attributes.h, janus/config.h)
├── contracts/        Cross-subsystem shared type definitions
├── _start/           Entry point — creates kernel.elf
├── kmain/            Kernel main — composition root, links subsystems
├── lib/              Shared utility libraries
├── core/             Shared kernel services (kio)
└── subsys/           Independent subsystems
    ├── boot/             Boot protocol handling
    ├── drivers/          Device drivers
    └── mm/               Memory management
```

Architecture-specific code lives inside the module that needs it, not in a
centralised `kernel/arch/` tree. A module's complete implementation — generic and
platform-specific — is co-located.

## Three-Tier Include Hierarchy

Every module with architecture-specific code uses three tiers of headers:

```
Tier 1 — Public API          subsys/foo/include/foo/bar.h           #include <foo/bar.h>
Tier 2 — Contract headers    subsys/foo/arch/include/arch/foo/bar.h #include <arch/foo/bar.h>
Tier 3 — Implementation      subsys/foo/arch/<ARCH>/include/arch/impl/foo/bar.h
```

Tier 1 is the stable public interface consumed by other modules. Tier 2 declares
`arch_*` functions and includes the Tier 3 header for the current target
architecture. Tier 3 is arch-specific internals and is never included directly from
outside the module.

## Dependency Graph

Auto-generated diagrams are written to `docs/generated/` during each CMake
configure run. See `docs/generated/deps-x86_64.md` and
`docs/generated/deps-aarch64.md`.
