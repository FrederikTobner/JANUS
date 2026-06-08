# Freestanding Environments

A C program can run in one of two environments. 
A *hosted* environment is the familiar one — the program runs under an operating system that provides the full C standard library: `malloc`, `printf`, file I/O, process control, and so on. 
A *freestanding* environment has none of this. 
The program runs without any underlying OS. 
There is no standard library, no runtime, no heap, and no assumption about the state of memory or hardware.

An OS kernel is the canonical example of a freestanding program. 
It is the software that *provides* the hosted environment for everything else; it cannot rely on one itself.

## What You Lose

In a freestanding C environment, the compiler guarantees only a handful of headers: `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>`, and `<float.h>`. 
These provide type definitions and constants but no functions.

Everything else — `printf`, `malloc`, `memcpy`, `strlen`, `fopen` — is absent. 
If the kernel needs any of these operations, it must implement them from scratch. 
This is why kernel projects maintain their own small utility libraries for memory operations, formatted output, and string handling.

## Compiler Flags

Building a freestanding binary requires telling the compiler to not link the standard library and to not assume a hosted runtime:

```bash
clang -ffreestanding -nostdlib -c kernel.c -o kernel.o
```

`-ffreestanding` tells the compiler not to assume that standard library functions have their standard semantics (it may otherwise optimise a loop into a `memset` call that does not exist). `-nostdlib` prevents the linker from pulling in libc or the default startup files.

## The Entry Point

A hosted C program enters at `main`, which is called by the C runtime startup code (`crt0`). 
In a freestanding program there is no `crt0`. 
The entry point is whatever the linker script or bootloader protocol defines — typically a symbol like `_start` written in assembly. 
This entry code sets up a stack, initialises any hardware state the C code expects, and then calls into the kernel's C entry function.

## Verification

A quick way to confirm that a toolchain is correctly configured for freestanding development is to compile a minimal program and verify that it produces a standalone object file with no external dependencies:

```c
void _start(void) {
    while (1) {
        __asm__ volatile ("hlt");
    }
}
```

```bash
clang -target x86_64-elf -ffreestanding -nostdlib -c test.c -o test.o
file test.o    # Should report: ELF 64-bit LSB relocatable, x86-64
```

Adding a call to `printf` and recompiling should produce an undefined-symbol error. 
If that is not the case, the toolchain is not properly configured for freestanding use.
