# Verifying Your Setup

Let's make sure everything actually works before we get three chapters in and discover your assembler is from 2003.

[!side]
This section takes 2 minutes. Do it. Future you will thank present you.
[/!side]

## Version Check

Run these commands and verify the versions:

```bash
clang --version      # ≥ 17.0
ninja --version      # Any recent version
nasm --version       # ≥ 2.15
cmake --version      # ≥ 3.20
qemu-system-x86_64 --version  # ≥ 6.2
lldb --version       # Any recent version
```

All tools should be found and display version information.

## Test: Compile Freestanding Code


[!side]
The `_start` symbol is the entry point for freestanding programs. The OS doesn't set this up—we do.
[/!side]

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
```

Expected output:

```
test.o: ELF 64-bit LSB relocatable, x86-64
```

If this works, your compiler is correctly configured for OS development.

> **The Crux: What is "Freestanding"?**
>
> Normal C programs are "hosted"—they run under an OS that provides `malloc()`, `printf()`, file I/O, etc. Our kernel is "freestanding"—no OS beneath us. We ARE the OS. This means:
>
> - No standard library (no `printf`, `malloc`, `strlen`)
> - No assumptions about the environment
> - We implement everything ourselves
>
> It's liberating and terrifying in equal measure.

Let's verify Clang can compile freestanding code for x86_64:

Try adding `printf("hello\n");` to the code above and recompile. Clang will complain that `printf` is undefined. There's no C standard library here. We're on our own.

This is expected! If `printf` somehow works, your environment isn't properly configured for freestanding development.

## Test: Assemble with NASM

```nasm
; test.asm
BITS 64
global _start

_start:
    hlt
    jmp _start
```

[!side]
`hlt` halts the CPU until the next interrupt. Without it, the CPU would execute random memory as code.
[/!side]

Assemble it:

```bash
nasm -f elf64 test.asm -o test_asm.o
file test_asm.o
```

Expected output:

```
test_asm.o: ELF 64-bit LSB relocatable, x86-64
```

## Test: QEMU

Launch QEMU to verify it works:

```bash
qemu-system-x86_64
```

You should see a QEMU window open with an error message like "No bootable device". That's expected—we haven't given it anything to boot yet.

Press Ctrl+C in the terminal to quit QEMU.

## Test: CMake

Create a minimal `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(test C)

add_executable(dummy test.c)
```

Generate build files:

```bash
cmake -B build -G Ninja
```

Expected output:

```
-- The C compiler identification is Clang 17.0.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Configuring done
-- Generating done
-- Build files have been written to: .../build
```

Clean up:

```bash
rm -rf build CMakeLists.txt test.c test.asm test.o test_asm.o
```

## All Tests Pass?

If all tests succeeded, you have a working TinyOS development environment. If any failed, review the error messages and consult the troubleshooting section in the previous chapter.

---

**Next: [Project Setup](project-setup.md)**
