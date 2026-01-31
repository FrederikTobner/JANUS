# JANUS

JANUS (Just ANother Unix-like System) is an educational x86-64 operating system kernel written in C17 and assembly, focusing on low-level hardware interaction and modular architecture.

## Quick Start

Building the kernel

```bash
# Build the kernel
cmake -B build 
cmake --build build
```

Creating a bootable ISO

```bash
cmake --build build --target iso 
```

Running in QEMU

```bash
cmake --build build --target run 
```

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
    ├── _start/           # Entry point (creates kernel.elf)
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
    └── x86_64/
        ├── include/arch/impl/example/  # Tier 3: Arch inline headers
        │   └── *.h
        └── *.c                         # Arch source files (optional)
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
- **Compiler**: Clang 17 or GCC with x86_64-elf cross-compilation support
- **Assembler**: NASM
- **Build System**: CMake 3.20+

## Documentation

Comprehensive documentation is in the [`Documentation/`](Documentation/) folder:

## Building

### Prerequisites

- **Clang 17+ or GCC 15+** - C compilers with x86_64-elf cross-compilation support
- **NASM** - Assembler for boot code and low-level kernel code
- **CMake 3.20+** - Build system generator
- **Ninja** - Fast build tool (recommended) or Make
- **QEMU** - x86_64 system emulator for testing
- **GRUB tools** - grub-pc-bin, xorriso, mtools (for creating bootable ISO images)
- **LLDB** - Debugger for kernel debugging (optional but recommended)

Installing prerequisites:

```bash
# Debian/Ubuntu
sudo apt install clang nasm cmake ninja-build qemu-system-x86 \
                 grub-pc-bin xorriso mtools lldb

# Arch Linux
sudo pacman -S clang nasm cmake ninja qemu-system-x86 \
               grub xorriso mtools lldb

# macOS
brew install llvm nasm cmake ninja qemu grub xorriso mtools
```

See [Documentation/Setup.md](Documentation/Setup.md) for detailed installation instructions.

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
