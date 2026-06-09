# JANUS — Overview

JANUS is a freestanding hobby kernel written in C17, targeting x86_64 and aarch64.
It is built as a collection of independent, well-defined modules with strict
layering enforced at compile time.

## Philosophy

**Educational Transparency.** Every architectural choice should be understandable
and documented. Where a design departs from the obvious approach, the reasoning is
recorded in code comments, commit messages, or these documents. References to
processor manuals and specifications are included where they add clarity.

**Modular Architecture.** JANUS is built as independent modules drawing on patterns
from the Linux kernel and LLVM. Each module has clear boundaries and explicit
dependencies. Circular dependencies are forbidden and enforced by CMake. Rather than
centralising all platform logic in a single `arch/` tree, each subsystem contains
its own `arch/` subdirectory, so a module's complete implementation — generic and
platform-specific — lives in one place.

**Public Structures Over Opaque Handles.** Kernel data structures are defined
publicly rather than hidden behind opaque pointer typedefs. In a freestanding
kernel, the benefits of opaque handles (API stability across shared-library
boundaries) do not apply, while the costs — pointer indirection, forced heap
allocation, degraded cache locality, and poor debugger visibility — are real.
Public structures allow stack allocation, embedding for cache-friendly layouts,
and full visibility in LLDB or GDB. The rare exceptions are hardware abstraction
boundaries where the underlying representation genuinely varies at runtime.

## Language and Compiler

C17 (`-std=gnu17`) is used for all kernel code, combined with architecture-specific
assembly for entry points and low-level hardware interaction. C17 provides
`_Static_assert`, `_Alignof`, and `_Noreturn` without experimental flags and enjoys
universal compiler support.

x86_64 assembly is written in NASM (Intel syntax). aarch64 assembly uses GAS
(standard ARM syntax). Inline GCC `asm` is used sparingly and only for
single-instruction helpers where a standalone file would be less clear.

Both GCC and Clang are supported and tested in CI:

| Preset | Compiler | Architecture |
|---|---|---|
| `x86_64-gcc` | GCC | x86_64 |
| `x86_64-clang` | Clang | x86_64 |
| `aarch64-gcc` | GCC | aarch64 |
| `aarch64-clang` | Clang | aarch64 |

All presets compile with `-Wall -Wextra -Werror -Wconversion -ffreestanding
-fno-builtin -nostdlib`.
