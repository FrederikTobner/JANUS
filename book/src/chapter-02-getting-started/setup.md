# Setting Up Your Environment

Operating system development requires a specialized toolchain. Unlike application development where you compile against an existing OS, we're building the OS itself. Unlike application development where you compile against an existing OS, we're building the OS itself—there's no standard library, no runtime, just bare metal.

What we will cover in this chapter:

- Essential tools for TinyOS development
- Installation across different platforms
- Verification that your environment works

## Prerequisites

This book assumes you have:

- **Working knowledge of C** (pointers, structs, memory management)
- **Basic assembly understanding** (registers, instructions, calling conventions)
- **Familiarity with CMake** (targets, subdirectories, basic commands)
- **Command-line proficiency** (shell navigation, build tools)

If these are new to you, spend time with foundational resources first. OS development builds on these skills—it doesn't teach them from scratch.

## Required Tools

### Compiler: Clang 17+

We use Clang because:

1. **Cross-compilation just works** — `clang -target x86_64-elf` and we're done. No spending an afternoon building binutils and GCC from source.
2. **Error messages are actually helpful** — "expected ';' before '}' token" beats GCC's cryptic template error novels.
3. **Consistent across platforms** — same behavior on Linux, macOS, and even WSL.

GCC works too (see Appendix B), but Clang makes our lives easier.

```bash
# Debian/Ubuntu
sudo apt install clang

# Arch Linux
sudo pacman -S clang

# Verify
clang --version
```

You should see output like:

```
clang version 17.0.0
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /usr/local/bin
```

**Alternative:** GCC 13+ with a cross-compiler targeting `x86_64-elf` works but requires building from source. See Appendix B for GCC setup.

### Build System: Ninja

Ninja is a fast, minimal build system. CMake will generate Ninja build files.

```bash
# Debian/Ubuntu
sudo apt install ninja-build

# Arch Linux
sudo pacman -S ninja

# Verify
ninja --version
```

You should see:

```
1.10.1
```

(Or a similar recent version)

**Alternative:** Make also works. Omit `-G Ninja` from CMake commands to use Make instead.

### Assembler: NASM

NASM assembles x86_64 boot code and low-level kernel code.

```bash
# Debian/Ubuntu
sudo apt install nasm

# Arch Linux
sudo pacman -S nasm

# Verify
nasm --version
```

You should see:

```
NASM version 2.15.05
```

### Emulator: QEMU

QEMU emulates a complete x86_64 system, letting us test without rebooting real hardware.

```bash
# Debian/Ubuntu
sudo apt install qemu-system-x86

# Arch Linux
sudo pacman -S qemu-system-x86

# Verify
qemu-system-x86_64 --version
```

You should see:

```
QEMU emulator version 6.2.0 (or newer)
Copyright (c) 2003-2021 Fabrice Bellard and the QEMU Project developers
```

### Debugger: LLDB

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

### Build Tools: CMake 3.20+

```bash
# Debian/Ubuntu
sudo apt install cmake

# Arch Linux
sudo pacman -S cmake

# Verify
cmake --version  # Must be 3.20 or higher
```

### Bootloader Tools: GRUB and ISO Creation

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

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y clang ninja-build nasm cmake \
                    qemu-system-x86 lldb git \
                    grub-pc-bin xorriso mtools
```

### Arch Linux

```bash
sudo pacman -S clang ninja nasm cmake \
               qemu-system-x86 lldb git \
               grub xorriso mtools
```

## Verifying Your Setup

Let's make sure everything actually works before we get three chapters in and discover your assembler is from 2003.

Run these commands and verify the versions match:

```bash
clang --version      # ≥ 17.0
ninja --version
nasm --version
cmake --version      # ≥ 3.20
qemu-system-x86_64 --version
lldb --version
```

### Test: Compile Freestanding Code

> **The Crux: What is "Freestanding"?**
>
> Normal C programs are "hosted"—they run under an OS that provides `malloc()`, `printf()`, file I/O, etc. Our kernel is "freestanding"—no OS beneath us. We ARE the OS. This means:

> - No standard library (no `printf`, `malloc`, `strlen`)
> - No assumptions about the environment
> - We implement everything ourselves
>
> It's liberating and terrifying in equal measure.

Let's verify Clang can compile freestanding code for x86_64:

```c
// test.c
void _start(void) {
    while (1) {
        __asm__ volatile ("hlt");
    }
}
```

Compile it:

```bash
clang -target x86_64-elf -ffreestanding -nostdlib \
      -c test.c -o test.o

# Verify it's ELF x86-64
file test.o
# Output: test.o: ELF 64-bit LSB relocatable, x86-64
```

If this works, your compiler is correctly configured for OS development.

You can try adding `printf("hello\n");` to the code above and recompile. Clang will complain that `printf` is undefined. There's no C standard library here. We're on our own.

### Test: Assemble with NASM

```nasm
; test.asm
BITS 64
global _start

_start:
    hlt
    jmp _start
```

```bash
nasm -f elf64 test.asm -o test_asm.o
file test_asm.o
# Output: test_asm.o: ELF 64-bit LSB relocatable, x86-64
```

### Test: QEMU

```bash
# Launch QEMU (will show "No bootable device", that's expected)
qemu-system-x86_64

# Ctrl+C to quit
```

If QEMU launches without errors, you're ready.

## Project Setup

Create a directory for your OS project:

```bash
mkdir -p TinyOS
cd TinyOS

# Initialize git repository
git init
```

We'll build the project incrementally throughout the book. Each chapter adds new components:

- Boot code (Chapter 2)
- Build system (Chapter 2)
- Physical memory manager (Chapter 3)
- Virtual memory with paging (Chapter 4)
- And more...

Don't worry about CMake configuration yet. We'll create the build system piece by piece as we understand what we're building.

## What We're Not Covering

This chapter focuses on *toolchain setup*, not:

- **Editor configuration**: Use whatever you prefer. The project includes `.clangd` for LSP-compatible editors (Language Server Protocol - provides autocomplete, go-to-definition, error highlighting) if you want it.
- **Git workflows**: Assumed knowledge.
- **Shell basics**: You should already know how to navigate directories and run commands.
- **C language tutorial**: We'll write plenty of C, but won't explain pointers or struct syntax.
- **CMake tutorials**: We'll use it extensively, but won't teach the basics.

We're here to build an OS, not learn prerequisite tools. If those feel unfamiliar, get comfortable with them first. You'll have a much better time.

## Troubleshooting

### Clang version too old

Your distribution might have an older Clang. Install from LLVM's official packages or use GCC as an alternative.

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

### Cross-compilation errors

Verify your flags in `cmake/TinyOSPlatform.cmake`. We need:

- `-target x86_64-elf` (Clang's built-in cross-compile)
- `-ffreestanding` (no hosted environment)
- `-nostdlib` (no standard library linking)

## What's Next

You now have a complete TinyOS development environment. In the next chapter, we'll write a bootloader and see our first kernel message.

---

**Next: [Summary](./summary.md)**
