# Installing Development Tools

> *"Give me six hours to chop down a tree and I will spend the first four sharpening the axe."*  
> — Abraham Lincoln

Operating system development requires specialized tools. Unlike application development where you compile against an existing OS, we're building the OS itself—there's no standard library, no runtime, just bare metal.

We need to set up the following tools:

[!side]
Why are we using LLVM tools instead of GCC?

LLVM generally produces better compiler error messages, but you can also use GCC if you prefer. Since Clang is largely a drop-in replacement for GCC, the code we write will be compatible with both compilers.
[/!side]

- Clang 17+
- NASM 2.15+
- CMake 3.20+ and Ninja
- QEMU 6.2+
- LLDB
- GRUB, xorriso, mtools

**Compiler: Clang 17+**

> TODO: Actually test compilation using GCC once we finished the book.

```bash
# Debian-based systems
sudo apt install clang

# Arch Linux
sudo pacman -S clang
```

**Assembler**

NASM assembles x86_64 boot code and low-level kernel code.

[!side]
NASM uses Intel syntax, e.g. `mov eax, 5`.

The GNU Assembler (GAS) uses AT&T syntax, e.g. `movl $5, %eax`.
[/!side]

```bash
# Debian-based systems
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm
```

**Build System: CMake and Ninja**

CMake generates build files that Ninja then uses to build TinyOS.

```bash
# Debian-based systems
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja
```

[!side]
Make would, of course, work as well.
Omit the `-G Ninja` from CMake commands to use Make instead.

However, we will only show how to use Ninja in this book. If you are not already familiar with Make, I recommend sticking with Ninja to avoid confusion.
[/!side]

**Emulator: QEMU**

QEMU emulates a complete x86_64 system, letting us test without rebooting real hardware.

```bash
# Debian-based systems
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86
```

**Debugger: LLDB**

LLDB debugs TinyOS by connecting to QEMU's GDB server.

```bash
# Debian-based systems
sudo apt install lldb

# Arch Linux
sudo pacman -S lldb
```

**Bootloader Tools: GRUB and ISO Creation**

We need GRUB tools to create bootable ISO images for testing.

```bash
# Debian-based systems
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools
```

`grub-mkrescue` creates a bootable ISO image with GRUB as the bootloader. `xorriso` and `mtools` are dependencies required by `grub-mkrescue`.

> TODO: Verify these are the correct package names. They may differ by distro.

To install all required tools in one go:

```bash
# Debian-based systems
sudo apt update
sudo apt install -y clang ninja-build nasm cmake \
                    qemu-system-x86 lldb \
                    grub-pc-bin xorriso mtools
# Arch Linux
sudo pacman -S clang ninja nasm cmake \
               qemu-system-x86 lldb \
               grub xorriso mtools
```

---

**Next: [Verification](verification.md)**
