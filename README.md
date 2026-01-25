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
└── kernel/           # kernel functionality
       ├── arch/             # Architecture specific layer
       ├── core/             # Core kernel functionality
       ├── boot/             # Multiboot2 boot loader
       ├── lib/              # Kernel libraries (arch, utility libraries)
       ├── mm/               # Memory management
       ├── drivers/          # Device drivers
       └── include/          # Global headers
```

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
