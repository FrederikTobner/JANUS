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

**Note:** When switching architectures, delete the build directory first: `rm -rf build`

## Project Structure

```
JANUS/
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
├── book/             # Book / tutorial of the project
├── Documentation/    # Technical documentation
└── kernel/           # Kernel functionality
    ├── include/          # Global headers (janus/types.h, etc.)
    ├── _start/           # Entry point (per-arch: x86_64/, aarch64/)
    ├── kmain/            # Kernel main - final assembly point
    ├── lib/              # Shared utility libraries
    └── subsys/           # Independent subsystems
        ├── boot/             # Boot protocol handling
        ├── drivers/          # Device drivers (with arch/)
        └── mm/               # Memory management
```

### Architecture Code

Architecture-specific code lives **inside each subsystem** that needs it, using a three-tier include model:

```
kernel/subsys/example/
├── include/example/                    # Tier 1: Public API
│   └── *.h
├── *.c                                 # Generic impl (optional)
└── arch/
    ├── include/arch/example/           # Tier 2: Contract headers
    │   └── *.h                         #   declares arch_* or includes Tier 3
    ├── shared/                         # Shared arch code (framebuffer, etc.)
    │   └── *.c
    ├── x86_64/
    │   ├── include/arch/impl/example/  # Tier 3: Arch inline headers
    │   │   └── *.h
    │   └── *.c                         # Arch source files
    └── aarch64/
        ├── include/arch/impl/example/  # Tier 3: Arch inline headers
        │   └── *.h
        └── *.c                         # Arch source files
```

**Flexibility:**

- **Generic layer**: Each public header can be header-only or have a `.c` file
- **Arch layer**: Each contract can include inline headers (Tier 3) or declare functions implemented in `.c` files
- **No 1:1 mapping required**: A public `X.h` can include arch contracts `Y.h` and `Z.h`; arch implementations can be split across multiple `.c` files

The only rule: Tier 2 contracts declare `arch_*` functions that get implemented either inline (Tier 3) or in arch `.c` files.

For a detailed explanation of the architecture (arch) layer structure and all supported patterns, see:
[Documentation/Code-Organization/Arch-Layer-Structure.md](Documentation/Code-Organization/Arch-Layer-Structure.md)

## Technology Stack

- **Language**: C17 (ISO/IEC 9899:2018) with GNU extensions
- **Compiler**: Clang 17 (x86_64), aarch64-linux-gnu-gcc (aarch64)
- **Assembler**: NASM (x86_64), GAS (aarch64)
- **Build System**: CMake 3.20+
- **Target Architectures**: x86_64, aarch64 (QEMU virt machine)

## Documentation

Comprehensive documentation is in the [`Documentation/`](Documentation/) folder:

## Building

### Prerequisites

**Common tools:**

- **CMake 3.20+** - Build system generator
- **Ninja** - Fast build tool (recommended) or Make
- **QEMU** - System emulator for testing
- **xorriso, mtools** - ISO creation tools

**For x86_64:**

- **Clang 17+** - C compiler
- **NASM** - Assembler
- **GRUB tools** - grub-pc-bin (for bootable ISO images)

**For aarch64:**

- **aarch64-linux-gnu-gcc** - Cross-compiler toolchain

**Optional:**

- **LLDB / GDB** - Debugger for kernel debugging

Installing prerequisites:

```bash
# Debian/Ubuntu (x86_64)
sudo apt install clang nasm cmake ninja-build qemu-system-x86 \
                 grub-pc-bin xorriso mtools lldb limine

# Debian/Ubuntu (aarch64)
sudo apt install gcc-aarch64-linux-gnu cmake ninja-build \
                 qemu-system-arm xorriso mtools gdb-multiarch limine

# Arch Linux (x86_64)
sudo pacman -S clang nasm cmake ninja qemu-system-x86 \
               grub xorriso mtools lldb limine

# Arch Linux (aarch64)
sudo pacman -S aarch64-linux-gnu-gcc cmake ninja \
               qemu-system-aarch64 xorriso mtools limine
```

See [Documentation/Setup.md](Documentation/Setup.md) for detailed installation instructions.

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
