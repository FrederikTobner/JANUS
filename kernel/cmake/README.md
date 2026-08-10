# kernel/cmake — Kernel-Only CMake Helpers

CMake helper functions used **only** by the freestanding kernel build. Nothing
here is included outside `kernel/CMakeLists.txt` (and files it transitively
`include()`s) — a hosted build (`tools/`, future `user/`) must never pick up
`-nostdlib`/`-ffreestanding` or the kernel-registry wrappers below.

## Registry.cmake

Thin wrapper around [`cmake/Registry.cmake`](../../cmake/Registry.cmake)'s
generic engine that sets up the `KERNEL` registry namespace and defines
`janus_register()`, `janus_validate_registry()`, `janus_write_mermaid_diagram()`.
Mirrors [`tools/cmake/Registry.cmake`](../../tools/cmake/Registry.cmake) exactly.

## CompilerFlags.cmake

Common kernel compiler flags (`-nostdlib -ffreestanding -fno-builtin
-fno-stack-protector`, warnings, build-type flags) plus
`janus_apply_compile_flags(TARGET)`. Includes the kernel ABI flags for the
current architecture from `arch/<arch>/CompilerFlags.cmake`.

## arch/\<arch\>/CompilerFlags.cmake

Kernel-only ABI flags per architecture (e.g. `-mcmodel=kernel`,
`-mno-red-zone`, `-mno-sse*` on x86_64; `-mgeneral-regs-only` on aarch64).
Deliberately separate from [`cmake/arch/<arch>/BootProtocols.cmake`](../../cmake/arch/x86_64/BootProtocols.cmake),
which is a shared (not kernel-only) fact consumed by `kernel/_start` and
`cmake/image/` alike.

## Target helpers

### `Library.cmake` — `janus_add_library(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a kernel library (STATIC, or INTERFACE when no sources are provided).

### `Core.cmake` — `janus_add_core(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a core service.

### `Contract.cmake` — `janus_add_contract(name [CONSUMERS ...])`

Used for creating a contract between some subsystems, that need to share a datastructure.

### `Subsystem.cmake` — `janus_add_subsys(name SOURCES ... [DEPENDENCIES ...])`

Used for creating a kernel subsystem.
Automatically detects an `arch/CMakeLists.txt` and calls `add_subdirectory(arch)`.

### `ArchSource.cmake` — `janus_add_arch_subsys(name SOURCES ...)`

Called from within `arch/CMakeLists.txt`.
Creates a `${name}_arch` static library with the three-tier include hierarchy as PUBLIC paths.

### `Executable.cmake` — `janus_add_kernel(TARGET ... LINKER_SCRIPT ... DEPENDENCIES ... OBJECTS ...)`

Links a kernel ELF from object libraries and dependencies.
Called from `_start/<arch>/` to produce the final `kernel-<protocol>.elf`.

### `Module.cmake`

Private helper (`_janus_add_module`) shared by `Library.cmake`, `Core.cmake`,
and `Subsystem.cmake`. Not meant to be used directly.

### `SmokeTests.cmake` — `janus_register_smoke_tests()`

Registers host-side CTest smoke tests that boot the built ISO in QEMU
headless and assert on serial output. Called from `cmake/image/Targets.cmake`
once the arch/protocol targets it depends on exist.
