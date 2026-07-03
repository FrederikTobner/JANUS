# Getting Started

## Dependencies

The following tools are required to configure, build, and run JANUS. 
Where a distribution-specific install command is given, Debian/Ubuntu and Arch Linux are covered other distributions may follow in the future. 

### Build System

CMake 3.20 or later is required, together with Ninja as the build backend.

```bash
sudo apt install cmake ninja-build   # Debian/Ubuntu
sudo pacman -S cmake ninja            # Arch Linux
```

### Compilers

Both GCC and Clang are supported for x86_64.
Cross-compiling for aarch64 requires the ARM64 GCC toolchain.
Clang can cross-compile natively but still needs the GNU linker from the cross-binutils package.

```bash
# x86_64
sudo apt install gcc clang
sudo pacman -S gcc clang

# aarch64 cross-toolchain
sudo apt install gcc-aarch64-linux-gnu llvm
sudo pacman -S aarch64-linux-gnu-gcc
```

### Assembler

NASM is required for x86_64 assembly. aarch64 uses GAS, which ships with the cross-compiler toolchain and requires no separate installation.

```bash
sudo apt install nasm
sudo pacman -S nasm
```

### Debugger

For the debugging workflow I use lldb, but you can use whatever debugger you prefer.
The repository includes a `.lldbinit` that automates connection to QEMU's debug server.

```bash
sudo apt install lldb gdb-multiarch
sudo pacman -S lldb gdb
```

### Emulator

QEMU is used to run the kernel in a virtual machine. The `qemu-system-x86` and `qemu-system-arm` packages are required for x86_64 and aarch64 respectively.

```bash
sudo apt install qemu-system-x86 qemu-system-arm
sudo pacman -S qemu-system-x86 qemu-system-aarch64
```

### ISO and Bootloader Tools

xorriso and mtools are needed to produce bootable ISO images for Limine.
Multiboot2 (x86_64 only) additionally requires GRUB.

```bash
sudo apt install xorriso mtools grub-pc-bin
sudo pacman -S xorriso mtools grub
```

Limine itself is fetched automatically by CMake via `FetchContent` and does not need to be installed separately.

### Language Server

clangd provides IDE features such as autocompletion and jump-to-definition. 
The repository includes a `.clangd` configuration file that points clangd at the
generated `compile_commands.json`.

```bash
sudo apt install clangd
sudo pacman -S clang   # includes clangd on Arch
```

## Building

The recommended workflow uses CMake presets, which bundle the generator, toolchain file, and build directory into a single named configuration:

```bash
cmake --preset x86_64-gcc
cmake --build --preset x86_64-gcc
```

Each preset writes to its own build directory (`build-x86_64-gcc/`, etc.), so all four configurations can coexist on disk simultaneously.

To build without presets, or to use a toolchain not covered by the presets:

```bash
# Native x86_64
cmake -B build -G Ninja
cmake --build build

# Explicit cross-compilation
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-gcc.cmake
cmake --build build
```

## Build Targets

| Target             | Description                                  |
|--------------------|----------------------------------------------|
| *(default)*        | Build `kernel.elf`                           |
| `iso`              | Create all ISOs for this platform            |
| `iso-limine`       | Limine ISO                                   |
| `iso-multiboot2`   | Multiboot2 ISO via GRUB (x86_64 only)        |
| `run-limine`       | Boot Limine ISO in QEMU                      |
| `run-multiboot2`   | Boot Multiboot2 ISO in QEMU (x86_64 only)    |
| `run-uefi`         | Boot in UEFI mode (requires OVMF)            |
| `debug-limine`     | Limine ISO with GDB server on `:1234`        |
| `debug-multiboot2` | Multiboot2 ISO with GDB server (x86_64 only) |
