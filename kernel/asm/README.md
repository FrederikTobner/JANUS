# asm — Inline Assembly Primitives

This directory is the **only** permitted location for `__asm__ volatile` in the JANUS kernel C code.

It provides a thin layer of header-only, architecture-specific wrappers around privileged hardware instructions. Each wrapper is `static __always_inline`, so the compiler emits the instruction directly at every call site with no function-call overhead.

## Purpose

Centralising inline assembly here makes the rest of the kernel easy to audit: a security or correctness review of hardware access only needs to inspect this directory. Any `__asm__` found outside of `kernel/asm/` is a policy violation.

## Layout

```
asm/
├── CMakeLists.txt          # Defines the 'janus_asm' INTERFACE target
├── x86_64/
│   └── include/
│       └── asm/
│           ├── cpu.h       # hlt · cli · sti
│           ├── io.h        # outb · inb
│           └── barriers.h  # mfence · lfence · sfence
└── aarch64/
    └── include/
        └── asm/
            ├── cpu.h       # wfi · msr daifset · msr daifclr
            ├── barriers.h  # dsb · isb · dmb
            ├── tlb.h       # tlbi vale1is · tlbi vmalle1is
            └── regs.h      # mrs/msr ttbr0_el1 · ttbr1_el1
```

The CMake build adds the architecture-specific `include/` directory to the include path of every consumer. Code that links against `janus_asm` can write `#include <asm/cpu.h>` and receive the correct implementation for the target architecture.

## Usage

```c
#include <asm/cpu.h>
#include <asm/barriers.h>

// In a Tier 3 header or library:
static __always_inline void arch_cpu_halt(void) { asm_cpu_hlt(); }
static __always_inline void arch_cpu_disable_interrupts(void) { asm_cpu_cli(); }
```

## Access Rules

| Layer | May use `asm/` headers? |
|---|---|
| `_start` (assembly files) | Not applicable — these are `.S` files, not inline C assembly |
| `kmain` | **No** — call subsystem public APIs instead |
| Subsystems (Tier 3 wrappers) | Yes — the canonical consumers |
| Libraries | Yes — e.g. `page_tables` needs barriers |
| asm layer itself | Is the source |

See [docs/src/project/architecture/privilege-model.md](../../docs/src/project/architecture/privilege-model.md) for the full privilege model.
