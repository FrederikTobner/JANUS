# Dependencies

This page lists the tools required to build, run, and debug JANUS. For general editor configuration and shell setup, consult your preferred tools' documentation.

## Target Architectures

JANUS supports two target architectures:

- **x86_64** — the default and primary development target.
- **aarch64** — ARM64, currently tested under QEMU's virt machine only.

## Build System

**CMake 3.20 or later** is required. JANUS uses CMake as its build system generator and relies on features introduced in 3.20 (notably presets).

```bash
# Debian/Ubuntu
sudo apt install cmake

# Arch Linux
sudo pacman -S cmake
```

**Ninja** is the recommended build backend. It is faster than Make for incremental builds and is what the CMake presets target.

```bash
# Debian/Ubuntu
sudo apt install ninja-build

# Arch Linux
sudo pacman -S ninja
```

## Compilers

Both GCC and Clang are supported. For x86_64, install either or both:

```bash
sudo apt install gcc clang        # Debian/Ubuntu
sudo pacman -S gcc clang          # Arch Linux
```

For aarch64 cross-compilation, you need the ARM64 GCC toolchain. Clang can cross-compile natively but still requires the GNU linker and objcopy from the cross-binutils package:

```bash
# GCC cross-compiler
sudo apt install gcc-aarch64-linux-gnu          # Debian/Ubuntu
sudo pacman -S aarch64-linux-gnu-gcc            # Arch Linux

# Clang (still needs GNU cross-binutils for linking)
sudo apt install clang llvm gcc-aarch64-linux-gnu
```

## Assembler

**NASM** is required for x86_64 assembly. aarch64 uses GAS, which ships with the cross-compiler toolchain.

```bash
sudo apt install nasm             # Debian/Ubuntu
sudo pacman -S nasm               # Arch Linux
```

## Debugger

**LLDB** is the primary debugger for x86_64. **GDB** (with multiarch support) is used for aarch64. The repository includes a `.lldbinit` with QEMU-specific settings.

```bash
sudo apt install lldb gdb-multiarch    # Debian/Ubuntu
sudo pacman -S lldb gdb                # Arch Linux
```

## Emulator

**QEMU** is used to run and test the kernel without bare-metal hardware.

```bash
# x86_64
sudo apt install qemu-system-x86      # Debian/Ubuntu
sudo pacman -S qemu-system-x86        # Arch Linux

# aarch64
sudo apt install qemu-system-arm      # Debian/Ubuntu
sudo pacman -S qemu-system-aarch64    # Arch Linux
```

## Bootloader Tools

**Limine** and **xorriso** are required to create bootable ISO images using the Limine protocol:

```bash
sudo apt install xorriso mtools       # Debian/Ubuntu
sudo pacman -S limine xorriso mtools  # Arch Linux
```

On Debian/Ubuntu, Limine must be built from source:

```bash
git clone https://github.com/limine-bootloader/limine.git
cd limine && make install
```

**GRUB** and **xorriso** are needed for Multiboot2 ISO images (x86_64 only):

```bash
sudo apt install grub-pc-bin xorriso mtools   # Debian/Ubuntu
sudo pacman -S grub xorriso mtools            # Arch Linux
```

## CMake Presets

The project provides four presets covering the full compiler-architecture matrix. Each preset uses its own build directory, so multiple configurations can coexist:

```bash
cmake --preset x86_64-gcc
cmake --preset x86_64-clang
cmake --preset aarch64-gcc
cmake --preset aarch64-clang
```

Build and run:

```bash
cmake --build --preset x86_64-gcc
cmake --build --preset x86_64-gcc --target run
```

## Building Without Presets

On an x86_64 host you can build directly with whatever compiler CMake finds:

```bash
cmake -B build -G Ninja
ninja -C build
ninja -C build run
```

To use a specific toolchain file:

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-clang.cmake
ninja -C build
```

When switching architectures without presets, delete the build directory first:

```bash
rm -rf build && cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/<arch>-<compiler>.cmake -G Ninja
```

## Build Targets

| Target            | Description                                        |
|-------------------|----------------------------------------------------|
| *(default)*       | Build `kernel.elf`                                 |
| `iso`             | Create all ISOs for this platform                  |
| `iso-limine`      | Create Limine ISO                                  |
| `iso-multiboot2`  | Create Multiboot2 ISO (GRUB, x86_64 only)          |
| `run-limine`      | Boot Limine ISO in QEMU                            |
| `run-multiboot2`  | Boot Multiboot2 ISO in QEMU (x86_64 only)          |
| `run-uefi`        | Boot in UEFI mode (requires OVMF)                  |
| `debug-limine`    | Boot Limine ISO with GDB server on `:1234`         |
| `debug-multiboot2`| Boot Multiboot2 ISO with GDB server (x86_64 only)  |

## Optional: Language Server

**clangd** provides IDE features (autocomplete, jump-to-definition). The repository includes a `.clangd` configuration file.

```bash
sudo apt install clangd               # Debian/Ubuntu
sudo pacman -S clang                   # Arch (includes clangd)
```
