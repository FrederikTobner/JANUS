# Setup Guide

This guide covers the essential dependencies needed to build and test JANUS. For general development environment setup (editor configuration, shell, etc.), refer to your preferred tools' documentation.

## Target Architecture

JANUS supports two target architectures:

- **x86_64** (default) - Standard PC/server architecture
- **aarch64** - ARM64 architecture (QEMU virt machine only, hardware support requires DTB parsing)

### Using CMake Presets (Recommended)

The project provides presets for all supported compiler/architecture combinations:

```bash
cmake --preset x86_64-gcc       # x86_64 with GCC
cmake --preset x86_64-clang     # x86_64 with Clang
cmake --preset aarch64-gcc      # aarch64 with GCC (cross-compile)
cmake --preset aarch64-clang    # aarch64 with Clang (cross-compile)
```

Each preset uses its own build directory (`build-<preset-name>/`), so you can have
multiple configurations at the same time without conflicts.

### Using Toolchain Files Directly

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-clang.cmake
```

### Native x86_64 Build (No Preset)

On an x86_64 host, you can simply run:

```bash
cmake -B build -G Ninja
ninja -C build
```

This defaults to x86_64 with whatever system compiler CMake finds. GCC users will
get a validation check to ensure the compiler targets the correct architecture.

**Note:** When switching architectures without presets, delete the build directory first:

```bash
rm -rf build && cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/<arch>-<compiler>.cmake -G Ninja
```

## Required Dependencies

### Build System

**CMake 3.20 or later**

CMake is the build system generator for JANUS.

```bash
# Debian/Ubuntu
sudo apt install cmake

# Arch Linux
sudo pacman -S cmake
```

**Ninja** (recommended)

Ninja is a fast build system that works well with CMake.

```bash
# Debian/Ubuntu
sudo apt install ninja-build

# Arch Linux
sudo pacman -S ninja
```

### Compiler (x86_64)

**GCC** or **Clang**

JANUS supports both GCC and Clang for x86_64 builds.

```bash
# Debian/Ubuntu
sudo apt install gcc clang

# Arch Linux
sudo pacman -S gcc clang
```

### Cross-Compiler (aarch64)

**aarch64-linux-gnu-gcc** and/or **Clang**

For building aarch64 kernels, you need either the ARM64 GCC cross-compilation
toolchain or Clang (which supports cross-compilation natively).

```bash
# GCC cross-compiler
# Debian/Ubuntu
sudo apt install gcc-aarch64-linux-gnu

# Arch Linux
sudo pacman -S aarch64-linux-gnu-gcc
```

```bash
# Clang (needs GNU cross-binutils for linking)
# Debian/Ubuntu
sudo apt install clang llvm gcc-aarch64-linux-gnu

# Arch Linux
sudo pacman -S clang llvm aarch64-linux-gnu-gcc
```

**Note:** Even when using Clang for aarch64, you need `aarch64-linux-gnu-ld` and
`aarch64-linux-gnu-objcopy` from the GNU cross-binutils package.

### Assembler

**NASM (Netwide Assembler)** - Required for x86_64

NASM is required for assembling x86_64 boot code and low-level architecture-specific code.

```bash
# Debian/Ubuntu
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm
```

**Note:** aarch64 uses GAS (GNU Assembler), which is included with the cross-compiler toolchain.

### Debugger

**LLDB** (for x86_64) or **GDB** (for aarch64)

```bash
# Debian/Ubuntu
sudo apt install lldb gdb-multiarch

# Arch Linux
sudo pacman -S lldb gdb
```

The project includes a [.lldbinit](../.lldbinit) configuration file with QEMU-specific settings and kernel debugging helpers.

### Emulator

**QEMU**

QEMU is used to run and test JANUS without bare metal hardware.

```bash
# x86_64 emulation
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86

# aarch64 emulation
# Debian/Ubuntu
sudo apt install qemu-system-arm

# Arch Linux
sudo pacman -S qemu-system-aarch64
```

### Bootloader Tools

**Limine and xorriso**

These tools are required to create bootable ISO images for Limine

```bash
# Debian/Ubuntu
sudo apt install xorriso mtools
git clone https://github.com/limine-bootloader/limine.git
cd limine
make install

# Arch Linux
sudo pacman -S limine xorriso mtools
```

**GRUB and xorriso** (x86_64 only)

These tools are required to create bootable ISO images for GRUB

```bash
# Debian/Ubuntu
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools
```

## Quick Start

### Building with Presets (Recommended)

```bash
# x86_64 with GCC
cmake --preset x86_64-gcc
cmake --build --preset x86_64-gcc

# x86_64 with Clang
cmake --preset x86_64-clang
cmake --build --preset x86_64-clang

# aarch64 with GCC (cross-compile)
cmake --preset aarch64-gcc
cmake --build --preset aarch64-gcc

# aarch64 with Clang (cross-compile)
cmake --preset aarch64-clang
cmake --build --preset aarch64-clang

# Run in QEMU
cmake --build --preset <preset> --target run
```

### Building without Presets

```bash
# x86_64 (default, uses system compiler)
cmake -B build -G Ninja
ninja -C build

# aarch64 with toolchain file
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-gcc.cmake
ninja -C build

# Run in QEMU
ninja -C build run
```

## Available Build Targets

| Target | Description |
|--------|-------------|
| `ninja` | Build kernel.elf |
| `ninja iso` | Create Limine ISO (primary) |
| `ninja iso-grub` | Create GRUB ISO (x86_64 only) |
| `ninja run` | Boot Limine ISO/kernel in QEMU |
| `ninja run-grub` | Boot GRUB ISO in QEMU (x86_64 only) |
| `ninja run-uefi` | Boot ISO in UEFI mode (requires OVMF) |
| `ninja debug` | Boot with GDB server on :1234 |
| `ninja debug-grub` | Boot GRUB ISO with GDB server (x86_64 only) |

## Optional Dependencies

### Language Server (for IDE support)

**clangd** (recommended)

If you want IDE features like autocomplete and jump-to-definition, install clangd. The project includes a [.clangd](../.clangd) configuration file.

```bash
# Debian/Ubuntu
sudo apt install clangd

# Arch Linux
sudo pacman -S clang  # Includes clangd already
```
