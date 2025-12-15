# Installing Development Tools

> *"Give me six hours to chop down a tree and I will spend the first four sharpening the axe."*  
> — Abraham Lincoln

Operating system development requires specialized tools. Unlike application development where you compile against an existing OS, we're building the OS itself—there's no standard library, no runtime, just bare metal.

[!side]
Total install time: 5-10 minutes on most systems. The actual learning starts in Chapter 3.
[/!side]

## Required Tools Overview

We need:

1. **Compiler** - Clang 17+ (or GCC 13+)
2. **Assembler** - NASM 2.15+
3. **Build System** - CMake 3.20+ and Ninja
4. **Emulator** - QEMU 6.2+
5. **Debugger** - LLDB (or GDB)
6. **Bootloader Tools** - GRUB, xorriso, mtools

## Compiler: Clang 17+

We use Clang because:

1. **Cross-compilation just works** — `clang -target x86_64-elf` and we're done. No spending an afternoon building binutils and GCC from source.
2. **Error messages are actually helpful** — "expected ';' before '}' token" beats GCC's cryptic template error novels.
3. **Consistent across platforms** — same behavior on Linux, macOS, and even WSL.

[!side]
The Linux kernel uses GCC. We on the other hand are using Clang. Both work. Clang is just easier for learning since cross-compilation is built-in.
[/!side]

### Installation

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang

# Verify
clang --version
```

Expected output:

```
clang version 17.0.0
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /usr/local/bin
```

**Alternative:** GCC 13+ with a cross-compiler targeting `x86_64-elf` works but requires building from source. See Appendix B for GCC setup.

## Assembler: NASM

NASM assembles x86_64 boot code and low-level kernel code.

[!side]
NASM uses Intel syntax (`mov eax, 5`). GAS (GNU Assembler) uses AT&T syntax (`movl $5, %eax`). Intel is more readable.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm

# Verify
nasm --version
```

Expected output:

```
NASM version 2.15.05
```

## Build System: CMake and Ninja

CMake generates build files, and Ninja executes them blazingly fast.

[!side]
Ninja rebuilds only changed files. On large projects it's noticeably faster than Make. For TinyOS the difference is negligible.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja

# Verify
cmake --version  # Must be 3.20 or higher
ninja --version
```

**Alternative:** Make also works. Omit `-G Ninja` from CMake commands to use Make instead.

## Emulator: QEMU

QEMU emulates a complete x86_64 system, letting us test without rebooting real hardware.

[!side]
QEMU can also expose a GDB server for debugging. We'll use this extensively to step through boot code.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86

# Verify
qemu-system-x86_64 --version
```

Expected output:

```
QEMU emulator version 6.2.0 (or newer)
Copyright (c) 2003-2021 Fabrice Bellard and the QEMU Project developers
```

## Debugger: LLDB

LLDB debugs TinyOS by connecting to QEMU's GDB server.

```bash
# Debian/Ubuntu
sudo apt install lldb

# Arch Linux
sudo pacman -S lldb

# Verify
lldb --version
```

**Alternative:** GDB works equally well. Use `gdb` instead of `lldb` in debugging commands.

## Bootloader Tools: GRUB and ISO Creation

We need GRUB tools to create bootable ISO images for testing.

```bash
# Debian/Ubuntu
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools

# macOS (requires Homebrew)
brew install grub xorriso mtools

# Verify
grub-mkrescue --version
xorriso --version
mformat --version
```

**What these do:**

- `grub-mkrescue` - Creates bootable ISO images with GRUB
- `xorriso` - ISO 9660 filesystem creator (required by grub-mkrescue)
- `mtools` - FAT filesystem tools (required by grub-mkrescue)

## Complete Installation

### Ubuntu/Debian (One Command)

```bash
sudo apt update
sudo apt install -y clang ninja-build nasm cmake \
                    qemu-system-x86 lldb git \
                    grub-pc-bin xorriso mtools
```

### Arch Linux (One Command)

```bash
sudo pacman -S clang ninja nasm cmake \
               qemu-system-x86 lldb git \
               grub xorriso mtools
```

## Common Issues

### Clang version too old

Your distribution might have an older Clang. Install from LLVM's official packages:

```bash
# Add LLVM repository (Ubuntu)
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 17
```

Or use GCC as an alternative.

### CMake can't find Clang

Explicitly specify the compiler:

```bash
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang
```

### NASM not found

Ensure it's in your PATH:

```bash
which nasm
```

If CMake still can't find it:

```bash
cmake -B build -G Ninja -DCMAKE_ASM_NASM_COMPILER=/usr/bin/nasm
```

### GRUB tools missing

Make sure you installed `grub-pc-bin` (not just `grub`):

```bash
dpkg -l | grep grub-pc-bin  # Ubuntu/Debian
pacman -Q | grep grub       # Arch
```

---

**Next: [Verification](verification.md)**
