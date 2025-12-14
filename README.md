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
├── kernel/           # Core kernel functionality
├── boot/             # Multiboot2 boot loader
├── arch/             # Architecture-specific code (x86_64)
├── lib/              # Kernel libraries (types, memory, buffer, fio)
├── mm/               # Memory management
├── drivers/          # Device drivers
├── include/          # Global headers
├── scripts/          # Build and utility scripts
├── cmake/            # CMake build modules
├── tools/            # Development tools
└── Documentation/    # Technical documentation
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

## Testing

**Planned:** TinyOS will use **BMUnit** (Bare Metal Unit), a testing framework inspired by Linux kernel's KUnit but adapted for our needs.

The planned approach will embed tests within modules (not in a separate `tests/` folder):

```
lib/buffer/
├── buffer.c              # Implementation
├── buffer_test.c         # Tests (planned)
└── include/lib/buffer.h
```

See [BMUnit-Testing.md](Documentation/Implementation/BMUnit-Testing.md) for the planned testing framework design.

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
```

Alternative build types:

```bash
# Debug build (default)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Release build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

## Running

```bash
# Run bootable ISO in QEMU
ninja -C build run

# Or manually:
qemu-system-x86_64 -cdrom build/tinyos.iso -boot d -serial stdio
```

### Debugging with LLDB

```bash
# Terminal 1: Start QEMU with debugger waiting
ninja -C build debug

# Terminal 2: Connect LLDB
lldb build/kernel.elf
(lldb) gdb-remote localhost:1234
(lldb) b kernel_main
(lldb) c
(lldb) register read
(lldb) n  # step to next line
(lldb) bt # show backtrace
```

See the [book](book/src/part-02-boot-process/creating-bootable-iso.md#proving-the-kernel-works-with-lldb) for detailed LLDB usage and debugging examples.

## License

This project is licensed under the [GNU AFFERO GENERAL PUBLIC LICENSE](LICENSE)
