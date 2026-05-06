# asm — Inline Assembly Primitives

This directory is the **only** permitted location for `__asm__ volatile` in the JANUS kernel C code.

It provides a thin layer of header-only, architecture-specific wrappers around privileged hardware instructions. Each wrapper is `static __always_inline`, so the compiler emits the instruction directly at every call site with no function-call overhead.

## Purpose

Centralising inline assembly here makes the rest of the kernel easy to audit: a security or correctness review of hardware access only needs to inspect this directory. Any `__asm__` found outside of `kernel/asm/` is a policy violation.

## Layout

```text
asm/
├── CMakeLists.txt          # Defines the 'janus_asm' INTERFACE target
├── include/
│   └── asm/
│       ├── cpu.h           # Public entry point
│       ├── io.h            # Public entry point
│       ├── barriers.h      # Public entry point
│       ├── regs.h          # Public entry point
│       ├── tlb.h           # Public entry point
│       └── capabilities.h  # Public capability surface and validation
└── arch/
    ├── include/
    │   └── arch/
    │       └── asm/
    │           ├── cpu.h       # Architecture contract
    │           ├── io.h        # Architecture contract
    │           ├── barriers.h  # Architecture contract
    │           ├── regs.h      # Architecture contract
    │           ├── tlb.h       # Architecture contract
    │           └── capabilities.h # Architecture capability contract
    ├── x86_64/
    │   └── include/
    │       └── arch/
    │           └── impl/
    │               └── asm/
    │                   ├── cpu.h       # hlt · cli · sti
    │                   ├── io.h        # outb · inb
    │                   ├── barriers.h  # mfence · lfence · sfence
    │                   ├── regs.h      # CR3 access
    │                   └── tlb.h       # invlpg
    └── aarch64/
        └── include/
            └── arch/
                └── impl/
                    └── asm/
                        ├── cpu.h       # wfi · msr daifset · msr daifclr
                        ├── barriers.h  # dsb · isb · dmb
                        ├── tlb.h       # tlbi vale1is · tlbi vmalle1is
                        └── regs.h      # mrs/msr ttbr0_el1 · ttbr1_el1
```

The CMake build adds three include roots for every `janus_asm` consumer: the public `include/` directory, the architecture contract directory `arch/include/`, and the active implementation directory `arch/<target>/include/`.

Code still includes `#include <asm/cpu.h>`, but the layers now have distinct responsibilities:

```c
<asm/cpu.h>            // Public asm_* wrappers and shared compositions
  -> <arch/asm/cpu.h>  // arch_asm_* contract functions
    -> <arch/impl/asm/cpu.h> // arch_asm_impl_* instruction primitives
```

This keeps public API names stable while making architecture requirements explicit.

Capabilities are exposed via `#include <asm/capabilities.h>`. This header validates that the selected architecture defines every required `ASM_ARCH_*` and `ASM_CAP_*` macro and enforces the single-architecture rule (`exactly one ASM_ARCH_* == 1`).

For architecture bring-up details, see [Adding a new architecture](ADDING_ARCH.md).

## Usage

```c
#include <asm/cpu.h>
#include <asm/barriers.h>

// In a Tier 3 header or library:
static __always_inline void arch_cpu_halt(void) { asm_cpu_halt_once(); }
static __always_inline void arch_cpu_disable_interrupts(void) { asm_cpu_disable_interrupts(); }

// Shared composed helper from asm layer:
// asm_cpu_halt_forever() disables interrupts then loops on halt-once.
```

## Access Rules

| Layer | May use `asm/` headers? |
| --- | --- |
| `_start` (assembly files) | Not applicable — these are `.S` files, not inline C assembly |
| `kmain` | **No** — call subsystem public APIs instead |
| Subsystems (Tier 3 wrappers) | Yes — the canonical consumers |
| Libraries | Yes — e.g. `page_tables` needs barriers |
| asm layer itself | Is the source |

See [docs/src/project/architecture/privilege-model.md](../../docs/src/project/architecture/privilege-model.md) for the full privilege model.
