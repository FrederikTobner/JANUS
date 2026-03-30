# Include Hierarchy

Every subsystem and library with architecture-specific code uses a three-tier include hierarchy. The tiers enforce a clean separation between the public interface that consumers depend on, the architecture contract that bridges generic and platform code, and the concrete implementation for a specific target.

## The Three Tiers

```text
┌──────────────────────────────────────────────────────────────────────┐
│  Tier 1 — Public API                                                 │
│  Location: subsys/foo/include/foo/                                   │
│  Include:  #include <drivers/cpu.h>                                  │
│  Consumers use this tier. It is architecture-agnostic and includes   │
│  the Tier 2 contract internally.                                     │
├──────────────────────────────────────────────────────────────────────┤
│  Tier 2 — Contract Headers                                           │
│  Location: subsys/foo/arch/include/arch/foo/                         │
│  Include:  #include <arch/drivers/cpu.h>                             │
│  Bridge between public API and arch implementation. Declares         │
│  arch_* functions and includes the Tier 3 header for the             │
│  current target architecture.                                        │
├──────────────────────────────────────────────────────────────────────┤
│  Tier 3 — Implementation                                             │
│  Location: subsys/foo/arch/<ARCH>/include/arch/impl/foo/             │
│  Include:  #include <arch/impl/drivers/cpu.h>                        │
│  The concrete implementation for x86_64, aarch64, etc.               │
│  Never included directly by consumers.                               │
└──────────────────────────────────────────────────────────────────────┘
```

## Patterns

The three tiers can be combined in several ways depending on where the logic lives:

- **All tiers header-only.** Every layer is inline code; no `.c` files are needed at any tier.
- **Generic header-only, arch `.c`.** The public API is inline helpers, but the architecture layer compiles platform-specific `.c` files.
- **Generic `.c`, arch header-only.** Generic code in `.c` files calls inline architecture helpers (e.g., `outb()` or `hlt()`).
- **Both `.c`.** Both the generic and architecture layers have compiled translation units.
- **Multi-file arch.** An architecture implementation can be split across multiple files when the platform logic is complex enough to warrant it.
- **Multi-contract.** A single public header can include several Tier 2 contracts if it aggregates multiple architecture concerns.

## Namespacing

Architecture headers are namespaced by their owning module to prevent collisions. The include path `arch/impl/drivers/cpu.h` is unambiguous even when multiple subsystems define their own `cpu.h`, because each carries its module name in the path.

Architecture implementations are not required to mirror the public API file structure. Use as many files as needed for clarity and separation of concerns — a single public header may map to several architecture-level files if the platform logic warrants it.

## CMake Integration

The build system automatically globs `.c` files from the correct architecture directory and configures include paths for all three tiers. Modules declare their architecture support through the `janus_add_subsys` or `janus_add_library` CMake helpers, which handle the tier-2 and tier-3 include path setup internally.
