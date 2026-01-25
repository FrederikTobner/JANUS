# Summary

You're now equipped with:

- **Cross-compilation toolchain** — Clang with x86_64-elf target support
- **Build system** — CMake and Ninja for automated builds
- **Assembler** — NASM for x86_64 assembly code
- **Emulator** — QEMU for testing without real hardware
- **Debugger** — LLDB for inspecting kernel execution
- **Bootloader** — GRUB for loading our kernel
- **Verified environment** — All tools tested and working
- **Project structure** — TinyOS directory initialized with git

Now we have fully set up the OS development environment: everything you need to write, build, debug, and boot a kernel.

Your development environment can now:

**Compile freestanding code:**

```bash
# Cross-compile bare metal C code
clang -target x86_64-elf -ffreestanding -c test.c
```

**Assemble x86_64 code:**

```bash
# Create 64-bit ELF object files
nasm -f elf64 boot.asm
```

**Build with automation:**

```bash
# Generate build files and compile
cmake -B build -G Ninja
ninja -C build
```

**Test in virtual hardware:**

```bash
# Boot kernel in QEMU emulator
qemu-system-x86_64 -cdrom kernel.iso
```

**Debug at machine level:**

```bash
# Connect debugger to running kernel
lldb kernel.elf
(lldb) gdb-remote :1234
```

[!side]
The hard part of the setup is over. Now comes the fun part: making the metal do what you want.
[/!side]

You're now equipped to tackle the boot process. No more setup, no more verification tests. Chapter 3 delivers the payoff: a working bootable kernel.

## Challenges

Before moving on, try these exercises to deepen your understanding of your toolchain:

1. **Compiler Target Exploration** (Research): Run `clang --print-targets` and examine the output. What's the difference between `x86_64-elf`, `x86_64-linux-gnu`, and `x86_64-pc-windows-msvc`? Why does the target matter for OS development?

2. **Freestanding vs Hosted** (Experiment): Write a simple C program that uses `printf`. Try compiling it with `-ffreestanding` vs without. What errors do you get, and why? Try fixing it by implementing your own `printf` that writes to a buffer instead of stdout.

3. **Assembly Deep Dive** (Implementation): Write a NASM program that:
 - Reserves 1024 bytes in the `.bss` section
 - Sets up a stack pointer to the top of that memory
 - Calls a C function you write
 - The C function uses that stack and returns a value
 - Compile both, link them, and verify with `readelf` that the stack is correctly aligned

4. **QEMU Machine Exploration** (Research): Run `qemu-system-x86_64 -machine help`. What machines can QEMU emulate? Research the differences between `pc` (default) and `q35`. Which is more modern, and what advantages does q35 have?

5. **Build System Comparison** (Research): CMake generates build files for Ninja. Research alternatives like Meson, Make, or build2. What are their tradeoffs? Why might we choose CMake+Ninja for kernel development? (Hint: consider dependency tracking, parallel builds, and cross-compilation support.)

6. **Custom Toolchain** (Advanced): Try building a true cross-compiler from source:
 - Build binutils for an `x86_64-elf` target
 - Build GCC with `--target=x86_64-elf --disable-hosted`
 - Compare the resulting toolchain to Clang's built-in cross-compilation
 - Which approach is simpler? Which gives more control?

---

**Next: [Chapter 3: The Boot Process](../chapter-03-boot-process/README.md)**
