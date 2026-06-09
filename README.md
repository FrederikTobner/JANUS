# JANUS

[![Build](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/build.yaml?branch=main&label=Build&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/build.yaml)
[![Tools](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/tools.yaml?branch=main&label=Tools&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/tools.yaml)
[![Docs](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/docs.yaml?branch=main&label=Docs&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/docs.yaml)
[![Doxygen](https://img.shields.io/github/actions/workflow/status/FrederikTobner/JANUS/doxygen.yaml?branch=main&label=Doxygen&logo=github)](https://github.com/FrederikTobner/JANUS/actions/workflows/doxygen.yaml)
[![Coverage](https://img.shields.io/badge/coverage-see%20workflow-lightgrey?logo=codecov)](https://github.com/FrederikTobner/JANUS/actions/workflows/tools.yaml)
[![C Standard](https://img.shields.io/badge/C-C17%20freestanding-blue?logo=c)](https://en.cppreference.com/w/c/17)
[![Architectures](https://img.shields.io/badge/arch-x86__64%20%7C%20aarch64-orange?logo=linux)](https://github.com/FrederikTobner/JANUS)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue?logo=gnu)](LICENSE)

JANUS (Just ANother Unix-like System) is an educational operating system kernel written in C17 and assembly, supporting **x86_64** and **aarch64** architectures. 
The kernel follows a monolithic design.

## Quick Start

Building the kernel

```bash
# Using presets (recommended):
cmake --preset x86_64-gcc        # or x86_64-clang, aarch64-gcc, aarch64-clang
cmake --build --preset x86_64-gcc

# Or manually (defaults to x86_64 with system compiler):
cmake -B build -G Ninja
ninja -C build
```

Available presets: `x86_64-gcc`, `x86_64-clang`, `aarch64-gcc`, `aarch64-clang`

Creating all bootable ISO's for the current architecture with the supported boot protocols

```bash
ninja -C build iso         # or: cmake --build --preset <preset> --target iso
```

Running in QEMU using limine (both supported by x86_64 and aarch64)

```bash
ninja -C build run-limine         # or: cmake --build --preset <preset> --target run-limine
```

See [Setup & Dependencies](docs/src/project/setup/dependencies.md) for detailed installation instructions.

## Documentation

The project documentation lives in [`docs/`](docs/src/SUMMARY.md) as a single mdbook, organised into three parts:

| Part | Purpose |
|------|---------|
| **Project** | JANUS architecture, subsystems, design decisions, setup, tools, coding style |
| **Wiki** | OS theory — hardware, boot, memory, tooling |
| **Tutorials** | Hands-on walkthroughs tied to the concepts |

## Project Structure

The project structure is inspired by Linux and LLVM, with a clear separation of concerns.
See [Module Structure](docs/src/project/architecture/module-structure.md) for a detailed explanation of the module layout and design principles.

### Architecture-specific Code

Architecture-specific code follows two patterns:

- **Inside each subsystem** (`subsys/foo/arch/<arch>/`) for subsystem-owned hardware logic — Tier 3 headers, drivers, and protocol implementations.
- **`kernel/asm/`** for raw hardware primitives (`__asm__ volatile`) — the only permitted location for inline assembly in kernel C code. Headers are consumed via `#include <asm/cpu.h>`, `#include <asm/barriers.h>`, etc.

See [Layer Model](docs/src/project/architecture/layer-model.md) for the rationale behind this design and the [Privilege Model](docs/src/project/architecture/privilege-model.md) for the access rules.

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
