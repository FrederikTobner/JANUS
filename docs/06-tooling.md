# Tooling

## CMake

CMake 3.20 or later with Ninja drives every aspect of the build.
Named presets in `CMakePresets.json` bundle the generator, toolchain file, and build directory into a single identifier, so the full configure-build-run cycle reduces to three commands:

```bash
cmake --preset x86_64-gcc
cmake --build --preset x86_64-gcc
cmake --build --preset x86_64-gcc --target run
```

The configure step generates a `compile_commands.json` in the build directory and copies it to the project root.
clangd and clang-tidy both read this file, so IDE autocomplete and static analysis are always in sync with the real build.
If editor completions appear stale after adding a new source file or changing include paths, re-running configure regenerates the file.

A full list of available CMake commands for inspecting cache variables, enabling verbose output, and cleaning the build directory is in [optional/02-cmake-reference.md](optional/02-cmake-reference.md).

## QEMU

The `run` build target invokes QEMU with the flags appropriate for the selected preset.
Driving the kernel through the build system is the recommended path because it keeps the QEMU flags, the ISO path, and the firmware selection consistent with how CI runs:

```bash
cmake --build --preset x86_64-gcc --target run
cmake --build --preset aarch64-clang --target run
```

QEMU's built-in GDB server is activated by the `debug-limine` target (see the LLDB section below).
The `-no-reboot` and `-d int,cpu_reset` flags are useful for catching triple faults before the VM resets; they are described in full in [optional/00-qemu-reference.md](optional/00-qemu-reference.md), along with the QEMU monitor commands for inspecting page tables and memory regions at runtime.

## LLDB

LLDB is the primary debugger.
The repository ships an `.lldbinit` that automates the connection sequence.
To start a debugging session, launch QEMU with the GDB server enabled, then attach LLDB:

```bash
# Terminal 1 — starts QEMU frozen, waiting for the debugger
cmake --build --preset x86_64-gcc --target debug-limine

# Terminal 2 — connects to the waiting VM
lldb
(lldb) gdb-remote localhost:1234
(lldb) b kernel_main
(lldb) c
```

The kernel binary contains full DWARF symbols when built with `Debug` or `RelWithDebInfo`.
Because there is no libc and no dynamic linker, the symbol table contains exactly the symbols defined in the codebase.
The entry point of the whole program then correspondsto the symbol `_start`, while the first C function, that is envoked within the kernel, is called `kernel_main`.

A complete LLDB command reference, including memory inspection, disassembly, and watchpoint commands, is in [optional/01-lldb-reference.md](optional/01-lldb-reference.md).

## Doxygen

Running `doxygen Doxyfile` from the project root generates the full API reference into `doxygen-out/html/`.
The HTML output is published automatically to GitHub Pages at `/JANUS/` on every push to `main`.

Well-formatted API documentation follows three rules: every public header file opens with a `@file` and `@brief` comment.
Every public structure carries a `@brief` and documents each field; and parameter and return value semantics are always stated explicitly rather than inferred from the function name.

## clang-tidy

clang-tidy performs static analysis against the kernel's actual compile database.
It runs automatically in CI on the `x86_64-clang` build cell and blocks merge if any check reports an error.
The active check set is configured in `.clang-tidy` at the project root. Suppressing a check with `// NOLINT` should be a last resort.
The preferred response to any finding is to fix its root cause.

To run clang-tidy locally before pushing, configure the `x86_64-clang` preset to produce a compile database and then invoke clang-tidy against the kernel sources.
The exact commands are in [optional/03-clang-tidy-reference.md](optional/03-clang-tidy-reference.md).
