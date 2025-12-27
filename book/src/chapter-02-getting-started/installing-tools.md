# Installing Development Tools

> *"Give me six hours to chop down a tree and I will spend the first four sharpening the axe."*  
> — Abraham Lincoln

Operating system development requires specialized tools. Unlike application development where you compile against an existing OS, we're building the OS itself—there's no standard library, no runtime, just bare metal.

[!side]
Total install time: 5-10 minutes on most systems. The actual learning starts in Chapter 3.
[/!side]

We need:

1. **Compiler** - Clang 17+ (or GCC 13+)
2. **Assembler** - NASM 2.15+
3. **Build System** - CMake 3.20+ and Ninja
4. **Emulator** - QEMU 6.2+
5. **Debugger** - LLDB (or GDB)
6. **Bootloader Tools** - GRUB, xorriso, mtools

**Compiler: Clang 17+**

We use Clang because:

1. **Cross-compilation just works** — `clang -target x86_64-elf` and we're done. No spending an afternoon building binutils and GCC from source.
2. **Error messages are actually helpful** — "expected ';' before '}' token" beats GCC's cryptic template error novels.
3. **Consistent across platforms** — same behavior on Linux, macOS, and even WSL.

[!side]
The Linux kernel uses GCC. We on the other hand are using Clang. Both work. Clang is just easier for learning since cross-compilation is built-in.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang
```

**Alternative:** GCC 13+ with a cross-compiler targeting `x86_64-elf` works but requires building from source. See Appendix B for GCC setup.

**Assembler**

NASM assembles x86_64 boot code and low-level kernel code.

[!side]
NASM uses Intel syntax (`mov eax, 5`). GAS (GNU Assembler) uses AT&T syntax (`movl $5, %eax`). Intel is more readable.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm
```

**Build System: CMake and Ninja**

CMake generates build files, and Ninja executes them blazingly fast.

[!side]
Ninja rebuilds only changed files. On large projects it's noticeably faster than Make. For TinyOS the difference is negligible.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja
```

**Alternative:** Make also works. Omit `-G Ninja` from CMake commands to use Make instead.

**Emulator: QEMU**

QEMU emulates a complete x86_64 system, letting us test without rebooting real hardware.

[!side]
QEMU can also expose a GDB server for debugging. We'll use this extensively to step through boot code.
[/!side]

```bash
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86
```

**Debugger: LLDB**

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

**Bootloader Tools: GRUB and ISO Creation**

We need GRUB tools to create bootable ISO images for testing.

```bash
# Debian/Ubuntu
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools
```

**What these do:**

- `grub-mkrescue` - Creates bootable ISO images with GRUB
- `xorriso` - ISO 9660 filesystem creator (required by grub-mkrescue)
- `mtools` - FAT filesystem tools (required by grub-mkrescue)

To install all required tools in one go:

**Ubuntu/Debian (One Command)**

```bash
sudo apt update
sudo apt install -y clang ninja-build nasm cmake \
                    qemu-system-x86 lldb \
                    grub-pc-bin xorriso mtools
```

**Arch Linux (One Command)**

```bash
sudo pacman -S clang ninja nasm cmake \
               qemu-system-x86 lldb \
               grub xorriso mtools
```

---

**Next: [Verification](verification.md)**
