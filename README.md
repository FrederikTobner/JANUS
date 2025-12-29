# TinyOS

An educational x86-64 operating system kernel written in C17 and assembly, focusing on low-level hardware interaction and modular architecture.

## Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd TinyOS

# Build the kernel
cmake -B build
cmake --build build

# Run in QEMU
qemu-system-x86_64 -kernel build/kernel.elf -serial stdio
```

## Project Structure

```
TinyOS/

├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
├── book/             # Book / tutorial of the project
├── Documentation/    # Technical documentation
└── kernel/           # kernel functionality
       ├── core/           # Core kernel functionality
       ├── boot/             # Multiboot2 boot loader
       ├── arch/             # Architecture-specific code (x86_64)
       ├── lib/              # Kernel libraries (types, memory, buffer, fio)
       ├── mm/               # Memory management
       ├── drivers/          # Device drivers
       └── include/          # Global headers
```

## Technology Stack

- **Language**: C17 (ISO/IEC 9899:2018) with GNU extensions
- **Compiler**: Clang 17
- **Debugger**: LLDB 17
- **Assembler**: NASM (Intel syntax)
- **Build System**: CMake 3.20+
- **Target**: x86_64-elf (bare metal)
- **Testing**: Planned - BMUnit (Bare Metal Unit testing framework)

## Documentation

Comprehensive documentation is in the [`Documentation/`](Documentation/) folder:

## Building

### Prerequisites

- **Clang 17+** - C compiler with x86_64-elf cross-compilation support
- **NASM** - Assembler for boot code and low-level kernel code
- **CMake 3.20+** - Build system generator
- **Ninja** - Fast build tool (recommended) or Make
- **QEMU** - x86_64 system emulator for testing
- **GRUB tools** - grub-pc-bin, xorriso, mtools (for creating bootable ISO images)
- **LLDB** - Debugger for kernel debugging (optional but recommended)

Installation:

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

### Build Commands

```bash
# Configure build
cmake -B build -G Ninja

# Build kernel
ninja -C build

# Create bootable ISO
ninja -C build iso

# Build and run in QEMU
ninja -C build run

# Build and run with debugger
ninja -C build debug

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
