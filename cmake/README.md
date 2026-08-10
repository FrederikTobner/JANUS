# cmake — Shared CMake Infrastructure

CMake modules shared by every build space (kernel, tools, and future user/stdlib
spaces): cross-compilation toolchains, platform/arch detection, the dependency
registry engine, and bootable-image assembly. Anything here must not assume it
is only ever used by the kernel — kernel-only helpers live in
[kernel/cmake/](../kernel/cmake/README.md) instead (see "Where does kernel-only
CMake code live?" below).

## Directory Layout

```
cmake/
├── toolchains/          # Toolchain files (set compiler + binutils per arch)
├── platform/            # Host/compiler/build-type detection (space-agnostic)
├── arch/                # Per-architecture SHARED facts (currently: supported boot protocols)
├── image/                # Bootable-ISO assembly: targets, QEMU config, boot-protocol staging
├── Registry.cmake       # Generic dependency-registry engine + Mermaid graph generator
└── README.md
```

## toolchains/

One file per `<arch>-<compiler>` combination. Sets `CMAKE_SYSTEM_NAME Generic`,
the cross compiler/binutils, and `JANUS_TARGET_ARCH`. Referenced directly by
`CMakePresets.json`. Reused as-is by any future cross-compiled space (kernel,
`user/`, `stdlib/`) — nothing here is kernel-specific.

## platform/

`Detection.cmake` — host OS / compiler-ID / build-type detection,
`CMAKE_EXPORT_COMPILE_COMMANDS`. Space-agnostic; sets `JANUS_PLATFORM_LOADED`.

## arch/

Per-architecture facts that more than one space needs at configure time —
currently just `BootProtocols.cmake`, which sets `JANUS_BOOT_PROTOCOLS`
(consumed by both `kernel/_start` and `cmake/image/`). This is deliberately
**not** where kernel ABI compiler flags live (e.g. `-mcmodel=kernel`,
`-mno-red-zone`) — those are kernel-only and live in
`kernel/cmake/arch/<arch>/CompilerFlags.cmake` instead.

## image/

Everything needed to turn a built kernel ELF (and, eventually, userspace
binaries) into a bootable ISO and run/debug it in QEMU:

- `Targets.cmake` — orchestrator: discovers boot-protocol modules, fetches
  Limine, defines the `iso`/`run-*`/`debug-*` targets.
- `arch/<arch>/Targets.cmake` — QEMU configuration and arch-specific special
  targets (`run-elf`, `run-uefi`, `debug-elf`).
- `boot/<proto>/Targets.cmake` — per-boot-protocol ISO staging (copies
  `kernel-<proto>.elf` and config templates into `iso/`) plus its `run`/`debug`
  targets.

## Registry.cmake

The generic, parameterised dependency-registry engine (`janus_registry_*`)
used to track registered targets, validate the subsystem-isolation rules, and
write a Mermaid dependency graph. It is intentionally generic — it knows
nothing about "kernel" or "tools". Each space wraps it with its own thin
`<space>/cmake/Registry.cmake` that calls `janus_registry_init(<NAME>)`:
[kernel/cmake/Registry.cmake](../kernel/cmake/Registry.cmake) and
[tools/cmake/Registry.cmake](../tools/cmake/Registry.cmake) are the two
existing wrappers (identical in shape) and the template for a future
`user/cmake/Registry.cmake`.

## Where does kernel-only CMake code live?

Anything that only makes sense for the freestanding kernel build — compiler
flags like `-nostdlib`/`-ffreestanding`, and the `janus_add_library`/
`janus_add_core`/`janus_add_subsys`/`janus_add_contract`/`janus_add_kernel`
target helpers — lives in `kernel/cmake/`, not here, and is only ever
`include()`-d from `kernel/CMakeLists.txt`. See
[kernel/cmake/README.md](../kernel/cmake/README.md) for that layer, and
`tools/cmake/` for the equivalent tools-only helpers.
