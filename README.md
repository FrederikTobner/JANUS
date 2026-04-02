# JANUS

JANUS (Just ANother Unix-like System) is an educational operating system kernel written in C17 and assembly, supporting **x86_64** and **aarch64** architectures. The project focuses on low-level hardware interaction and modular, multi-architecture design.
The kernel follows a mollithic design and is freestanding.

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
ninja -C build run-limine         # or: cmake --build --preset <preset> --target run
```

See [Setup & Dependencies](docs/src/project/setup/dependencies.md) for detailed installation instructions.

## Documentation

The project documentation lives in [`docs/`](docs/src/SUMMARY.md) as a single mdbook, organised into four parts:

| Part | Purpose |
|------|---------|
| **Wiki** | OS theory — hardware, boot, memory, tooling |
| **Tutorials** | Hands-on walkthroughs tied to the concepts |
| **Reference** | JANUS architecture, subsystems, design decisions |
| **Project** | Coding style, terminology, build setup, philosophy |

## Project Structure

The project structure is inspired by Linux and LLVM, with a clear separation of concerns.
See [Module Structure](docs/src/reference/architecture/module-structure.md) for a detailed explanation of the module layout and design principles.

### Architecture-specific Code

Architecture-specific code lives **inside each subsystem** that needs it.
See [Layer Model](docs/src/reference/architecture/layer-model.md) for the rationale behind this design and the supported patterns.

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
