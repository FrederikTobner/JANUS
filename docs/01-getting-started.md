# Getting Started

## Dependencies

**Build system:** CMake 3.20+ and Ninja.

```bash
# Debian/Ubuntu
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja
```

**Compilers:** GCC and Clang for x86_64; cross-toolchain for aarch64.

```bash
# Debian/Ubuntu
sudo apt install gcc clang gcc-aarch64-linux-gnu llvm

# Arch Linux
sudo pacman -S gcc clang aarch64-linux-gnu-gcc
```

**Assembler:** NASM is required for x86_64. aarch64 uses GAS from the cross-toolchain.

```bash
sudo apt install nasm           # Debian/Ubuntu
sudo pacman -S nasm             # Arch Linux
```

**Debugger:** LLDB for x86_64, GDB multiarch for aarch64.

```bash
sudo apt install lldb gdb-multiarch
sudo pacman -S lldb gdb
```

**Emulator:** QEMU.

```bash
sudo apt install qemu-system-x86 qemu-system-arm
sudo pacman -S qemu-system-x86 qemu-system-aarch64
```

**ISO tools:** xorriso and mtools (Limine); GRUB additionally for Multiboot2.

```bash
sudo apt install xorriso mtools grub-pc-bin
sudo pacman -S xorriso mtools grub
```

Limine is fetched automatically by CMake via `FetchContent`.

**Language server (optional):** clangd — the repository includes a `.clangd` config.

## Building

Configure and build using CMake presets:

```bash
cmake --preset x86_64-gcc
cmake --build --preset x86_64-gcc
```

Each preset writes to its own build directory (`build-x86_64-gcc/`, etc.), so
multiple configurations coexist without conflict.

Without presets:

```bash
cmake -B build -G Ninja
cmake --build build
```

Cross-compile for aarch64:

```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-gcc.cmake
cmake --build build
```

## Build Targets

| Target | Description |
|---|---|
| *(default)* | Build `kernel.elf` |
| `iso` | Create all ISOs for this platform |
| `iso-limine` | Limine ISO |
| `iso-multiboot2` | Multiboot2 ISO (GRUB, x86_64 only) |
| `run-limine` | Boot Limine ISO in QEMU |
| `run-multiboot2` | Boot Multiboot2 ISO in QEMU (x86_64 only) |
| `run-uefi` | Boot in UEFI mode (requires OVMF) |
| `debug-limine` | Limine ISO with GDB server on `:1234` |
| `debug-multiboot2` | Multiboot2 ISO with GDB server (x86_64 only) |
