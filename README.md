# JANUS

JANUS (Just ANother Unix-like System) is an educational operating system kernel written in C17 and assembly, supporting **x86_64** and **aarch64** architectures. The project focuses on low-level hardware interaction and modular, multi-architecture design.

## Quick Start

Building the kernel

```bash
# x86_64 (default)
cmake -B build -G Ninja
ninja -C build

# aarch64
cmake -B build -DJANUS_TARGET_ARCH=aarch64 -G Ninja
ninja -C build
```

Creating a bootable ISO

```bash
ninja -C build iso
```

Running in QEMU

```bash
ninja -C build run
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
