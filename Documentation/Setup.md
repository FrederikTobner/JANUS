# Setup Guide

This guide covers the essential dependencies needed to build and test JANUS. For general development environment setup (editor configuration, shell, etc.), refer to your preferred tools' documentation.

## Target Architecture

JANUS supports two target architectures:

- **x86_64** (default) - Standard PC/server architecture
- **aarch64** - ARM64 architecture (QEMU virt machine only, hardware support requires DTB parsing)

Set the target architecture when configuring CMake:

```bash
# x86_64 (default)
cmake -B build -DJANUS_TARGET_ARCH=x86_64 -G Ninja

# aarch64
cmake -B build -DJANUS_TARGET_ARCH=aarch64 -G Ninja
```

**Important:** When switching between architectures, you must delete and recreate the build directory to avoid CMake cache conflicts:

```bash
rm -rf build && cmake -B build -DJANUS_TARGET_ARCH=<arch> -G Ninja
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

**Clang 17 or later**

JANUS requires Clang for C17 support and cross-compilation to x86_64-elf.

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang
```

**Why Clang?** We use Clang for its excellent cross-compilation support, consistent behavior across platforms, and superior diagnostics.

### Cross-Compiler (aarch64)

**aarch64-linux-gnu-gcc**

For building aarch64 kernels, you need the ARM64 cross-compilation toolchain.

```bash
# Debian/Ubuntu
sudo apt install gcc-aarch64-linux-gnu

# Arch Linux
sudo pacman -S aarch64-linux-gnu-gcc
```

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

### Building for x86_64

```bash
# Configure and build
rm -rf build
cmake -B build -DJANUS_TARGET_ARCH=x86_64 -G Ninja
ninja -C build

# Run in QEMU
ninja -C build run
```

### Building for aarch64

```bash
# Configure and build
rm -rf build
cmake -B build -DJANUS_TARGET_ARCH=aarch64 -G Ninja
ninja -C build

# Run in QEMU (virt machine)
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
