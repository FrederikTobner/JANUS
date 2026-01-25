# Setup Guide

This guide covers the essential dependencies needed to build and test JANUS. For general development environment setup (editor configuration, shell, etc.), refer to your preferred tools' documentation.

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

### Compiler

**Clang 17 or later**

JANUS requires Clang for C17 support and cross-compilation to x86_64-elf.

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang
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
```

### Debugger

**LLDB**

LLDB is used for debugging JANUS in QEMU.

```bash
# Debian/Ubuntu
sudo apt install lldb

# Arch Linux
sudo pacman -S lldb
```

The project includes a [.lldbinit](../.lldbinit) configuration file with QEMU-specific settings and kernel debugging helpers.

### Emulator

**QEMU (x86_64 system emulation)**

QEMU is used to run and test JANUS without bare metal hardware.

```bash
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86
```

### Bootloader Tools

**GRUB and xorriso**

These tools are required to create bootable ISO images.

```bash
# Debian/Ubuntu
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools
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
```
