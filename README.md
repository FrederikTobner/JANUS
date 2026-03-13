# JANUS

JANUS (Just ANother Unix-like System) is an educational operating system kernel written in C17 and assembly, supporting **x86_64** and **aarch64** architectures. The project focuses on low-level hardware interaction and modular, multi-architecture design.

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

Creating a bootable ISO

```bash
ninja -C build iso         # or: cmake --build --preset <preset> --target iso
```

Running in QEMU

```bash
ninja -C build run         # or: cmake --build --preset <preset> --target run
```

See [Documentation/Setup.md](Documentation/Setup.md) for detailed installation instructions.

## Project Structure

The project is structure is inspired by Linux and LLVM, with a clear separation of concerns.
See [Documentation/Code-Organization/Module-Structure.md](Documentation/Code-Organization/Module-Structure.md) for a detailed explanation of the module structure and design principles.

### Architecture-specific Code

Architecture-specific code lives **inside each subsystem** that needs it.
For a detailed explanation of the rationale behind this design and the supported patterns, see [Documentation/Code-Organization/Arch-Layer-Structure.md](Documentation/Code-Organization/Arch-Layer-Structure.md).

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
