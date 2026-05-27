# Tools Standards

## Language Standard

All tool code is written in C11 (ISO/IEC 9899:2011). C11 is chosen for
consistency with the kernel — anyone comfortable with the kernel source will
find the same `_Static_assert`, fixed-width integer types, and compound
literal syntax. Tool tests are written in C++17, which is the minimum version
required by GoogleTest 1.14.

## Compiler Support

Tool builds are not tied to a cross-toolchain. Any C11-capable host compiler
is accepted. The CI matrix currently uses the same GCC and Clang versions as
the kernel presets, so compiler parity is maintained without extra effort.

Warnings are compiled with `-Wall -Wextra`. `-Wpedantic` is applied to tool
libraries, where stricter conformance is easier to guarantee than in tool
executables that may call variadic POSIX interfaces.

## Coding Conventions

Tool code follows the same conventions as the kernel wherever they apply to
hosted code.

**Integers.** Use `int32_t`, `uint32_t`, `size_t`, and other fixed-width or
semantically appropriate types. Plain `int` is acceptable only for values
whose width genuinely does not matter (e.g. boolean flags, loop counters
bounded by `int32_t` quantities).

**Static declarations before definitions.** Every translation unit lists all
`static` function forward declarations near the top, with full Doxygen doc
comments. Public (non-static) function definitions follow, then static
definitions at the bottom. This keeps the contract — what a file exposes and
what it hides — visible at a glance.

**Doc comments.** Public API declarations use `@file`, `@brief`, `@param`,
and `@return` tags. Internal (`static`) helpers use `@brief` at minimum.
Inline comments are reserved for non-obvious logic; self-evident code is left
uncommented.

**No section banners.** Section separator lines (`// ─────────────`) are not
used. Structure is conveyed through declaration order and doc comments, not
decorative ASCII.

## Build System Conventions

Each tool library lives under `tools/lib/<name>/` with the standard layout:

```
tools/lib/<name>/
  CMakeLists.txt
  include/<name>/<name>.h   ← public header(s)
  src/                      ← implementation + internal headers
```

Each tool executable lives under `tools/<name>/` with:

```
tools/<name>/
  CMakeLists.txt
  src/                      ← sources and private CMakeLists.txt
  tests/                    ← optional GoogleTest suite
```

Libraries register with `tools_add_lib()`; executables register with
`tools_add_tool()`. Both helpers live in `tools/cmake/` and automatically
record the target in the tools registry so that the dependency diagram is
kept up to date at configure time.

