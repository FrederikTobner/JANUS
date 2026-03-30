# Standards

JANUS targets C17 (ISO/IEC 9899:2018) for all kernel code, combined with assembly for platform-specific entry points and low-level hardware interaction. The choice of C17 is deliberate: it is the most recent revision of the language that enjoys universal compiler support without requiring experimental flags, and it provides the `_Static_assert`, `_Alignof`, and `_Noreturn` facilities that a freestanding kernel benefits from.

## Assembly

x86_64 assembly is written in NASM using Intel syntax. aarch64 assembly is written in GAS using the standard ARM syntax. Inline assembly (GCC extended `asm`) is used sparingly and only where a standalone `.asm` / `.S` file would be less clear — typically for single-instruction helpers like `outb` or `hlt` that benefit from being inlined at the call site.

## Compiler Support

Both GCC and Clang are supported and tested via CI on every commit. The project provides four CMake presets that cover the full matrix:

| Preset            | Compiler | Architecture |
|-------------------|----------|-------------|
| `x86_64-gcc`      | GCC      | x86_64      |
| `x86_64-clang`    | Clang    | x86_64      |
| `aarch64-gcc`     | GCC      | aarch64     |
| `aarch64-clang`   | Clang    | aarch64     |

All presets compile with `-Wall -Wextra -Werror -Wconversion`, ensuring that warnings are treated as errors across the entire codebase. The freestanding flags (`-ffreestanding -fno-builtin -nostdlib`) are applied globally.
