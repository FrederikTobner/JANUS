# cmake — Shared CMake Infrastructure

CMake modules shared by every build space (kernel, tools, and future user/stdlib
spaces): cross-compilation toolchains, platform/arch detection, the dependency
registry engine, and bootable-image assembly. Anything here must not assume it is only ever used by the kernel. 
Kernel-only helpers live in [kernel/cmake/](../kernel/cmake/README.md) instead.

## Directory Layout

```
cmake/
├── arch/                # Per-architecture SHARED facts (currently: supported boot protocols)
├── image/               # Bootable-ISO assembly: targets, QEMU config, boot-protocol staging
├── platform/            # Host/compiler/build-type detection (space-agnostic)
├── toolchains/          # Toolchain files (set compiler + binutils per arch)
├── Registry.cmake       # Generic dependency-registry engine + Mermaid graph generator
└── README.md
```

## toolchains/

One file per `<arch>-<compiler>` combination. Sets `CMAKE_SYSTEM_NAME Generic`,
the cross compiler/binutils, and `JANUS_TARGET_ARCH`. 
Referenced directly by `CMakePresets.json`. 
Reused as-is by any future cross-compiled space (kernel, `user/`, `stdlib/`)

## platform/

`Detection.cmake` — host OS / compiler-ID / build-type detection, CMAKE_EXPORT_COMPILE_COMMANDS`. 

## arch/

Per-architecture facts that more than one space needs at configure time.
Currently just `BootProtocols.cmake`, which sets `JANUS_BOOT_PROTOCOLS` (consumed by both `kernel/_start` and `cmake/image/`). 
This is deliberately **not** where kernel ABI compiler flags live (e.g. `-mcmodel=kernel`, `-mno-red-zone`). 
Those are kernel-only and live in kernel/cmake/arch/<arch>/CompilerFlags.cmake` instead.

## image/

Everything needed to turn a built kernel ELF (and, eventually, userspace binaries) into a bootable ISO and run/debug it in QEMU:

- `Targets.cmake` — orchestrator: discovers boot-protocol modules, fetches Limine, defines the `iso`/`run-*`/`debug-*` targets.
- `arch/<arch>/Targets.cmake` — QEMU configuration and arch-specific special targets (`run-elf`, `run-uefi`, `debug-elf`).
- `boot/<proto>/Targets.cmake` — per-boot-protocol ISO staging (copies `kernel-<proto>.elf` and config templates into `iso/`) plus its `run`/`debug`
  targets.

## Registry.cmake

Generic, parameterised dependency-registry engine (`janus_registry_*`) used to track registered targets, validate the subsystem-isolation rules, and write a Mermaid dependency graph, per project space. 
Each space wraps it with its own thin `<space>/cmake/Registry.cmake` that calls `janus_registry_init(<NAME>)`.

