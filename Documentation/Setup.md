# Setup Guide

This guide covers the essential dependencies needed to build and test TinyOS. For general development environment setup (editor configuration, shell, etc.), refer to your preferred tools' documentation.

## Required Dependencies

### Build System

**CMake 3.20 or later**

CMake is the build system generator for TinyOS.

```bash
# Debian/Ubuntu
sudo apt install cmake

# Arch Linux
sudo pacman -S cmake

# macOS
brew install cmake

# Verify installation
cmake --version  # Should show 3.20 or higher
```

### Compiler

**Clang 17 or later**

TinyOS requires Clang for C17 support and cross-compilation to x86_64-elf.

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang

# macOS (via Homebrew)
brew install llvm

# Verify installation
clang --version  # Should show 17.0 or higher
```

**Why Clang?** We use Clang for its excellent cross-compilation support, consistent behavior across platforms, and superior diagnostics.

### Assembler

**NASM (Netwide Assembler)**

NASM is required for assembling x86_64 boot code and low-level architecture-specific code.

```bash
# Debian/Ubuntu
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm

# macOS
brew install nasm

# Verify installation
nasm --version
```

### Debugger

**LLDB**

LLDB is used for debugging TinyOS in QEMU.

```bash
# Debian/Ubuntu
sudo apt install lldb

# Arch Linux
sudo pacman -S lldb

# macOS
brew install llvm  # Includes lldb

# Verify installation
lldb --version
```

The project includes a [.lldbinit](../.lldbinit) configuration file with QEMU-specific settings and kernel debugging helpers.

### Emulator

**QEMU (x86_64 system emulation)**

QEMU is used to run and test TinyOS without bare metal hardware.

```bash
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86

# macOS
brew install qemu

# Verify installation
qemu-system-x86_64 --version
```

### Bootloader Tools

**GRUB and xorriso**

These tools are required to create bootable ISO images.

```bash
# Debian/Ubuntu
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools

# macOS
brew install grub xorriso mtools

# Verify installation
grub-mkrescue --version
xorriso --version
```

## Optional Dependencies

### Language Server (for IDE support)

**clangd** (recommended)

If you want IDE features like autocomplete and jump-to-definition, install clangd. The project includes a [.clangd](../.clangd) configuration file.

```bash
# Debian/Ubuntu
sudo apt install clangd

# Arch Linux
sudo pacman -S clang  # Includes clangd

# macOS
brew install llvm  # Includes clangd
```

## Quick Start

Once you have the required dependencies installed:

```bash
# Clone the repository (if not already done)
git clone <repository-url>
cd TinyOS

# Configure the build
cmake -B build -G Ninja

# Build the kernel
ninja -C build

# Create bootable ISO
ninja -C build iso

# Run in QEMU
ninja -C build run

# Or run with debugging support
ninja -C build debug
# In another terminal:
lldb build/kernel.elf -o "gdb-remote localhost:1234"
```

## Verification

To verify your setup is complete:

```bash
# Check all required tools
cmake --version            # ≥ 3.20
clang --version            # ≥ 17.0
nasm --version
lldb --version
qemu-system-x86_64 --version
grub-mkrescue --version
xorriso --version

# Try to configure the project
cmake -B build -G Ninja

# Build and test
ninja -C build
ninja -C build iso

# Run the kernel
ninja -C build run
# You should see QEMU start and the kernel boot (blank screen is expected)
# Press Ctrl+C to exit
```

If all commands succeed, you're ready to start development!

## Troubleshooting

### CMake version too old

If your distribution provides an older CMake version, you can:

- Install from [Kitware's APT repository](https://apt.kitware.com/) (Ubuntu/Debian)
- Download binaries from [cmake.org](https://cmake.org/download/)
- Use pip: `pip3 install cmake`

### Clang not found or too old

Ensure Clang 17+ is in your PATH. On some systems, you may need to explicitly specify the compiler:

```bash
cmake -B build -DCMAKE_C_COMPILER=clang-17
```

### NASM not found

Verify NASM is in your PATH:

```bash
which nasm
```

If CMake can't find it, specify explicitly:

```bash
cmake -B build -DCMAKE_ASM_NASM_COMPILER=/usr/bin/nasm
```

### grub-mkrescue not found

Ensure GRUB tools are installed. On Ubuntu/Debian, you specifically need `grub-pc-bin` (for BIOS boot support), not just `grub-common`:

```bash
sudo apt install grub-pc-bin xorriso mtools
```

On some systems, grub-mkrescue may be in a non-standard location. Find it with:

```bash
which grub-mkrescue
```

### Cross-compilation issues

TinyOS targets `x86_64-elf`, which is a freestanding environment. If you get linker errors about missing standard library functions, verify that your CMakeLists.txt uses the correct flags:

- `-nostdlib` - Don't link standard libraries
- `-ffreestanding` - Freestanding environment
- `-target x86_64-elf` - Target triple

These are configured automatically in [cmake/TinyOSPlatform.cmake](../cmake/TinyOSPlatform.cmake).

## Next Steps

After setup is complete:

1. Read the [documentation overview](README.md)
2. Review [Core-Principles/Philosophy.md](Core-Principles/Philosophy.md)
3. Understand the [module structure](Code-Organization/Module-Structure.md)
4. Start coding with the [coding style guide](Implementation/Coding-Style.md)
