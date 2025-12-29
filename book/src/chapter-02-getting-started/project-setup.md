# Project Setup

Now that your tools are installed and verified, let's create the project structure.

[!side]
We build incrementally. Each chapter adds structure as we need it, not all upfront.
[/!side]

```bash
mkdir -p TinyOS
cd TinyOS
mkdir -p kernel cmake
cd kernel
mkdir -p boot core
```

This will lead to the following structure:

```
TinyOS/
├── cmake/
└── kernel/
    ├── boot/
    └── core/
```

In `cmake/`, we'll place CMake configuration files. In `kernel/`, we'll put all kernel-related code, with `boot/` for bootloader code and `core/` for the main kernel functionality.

Throughout this book, we'll use these compiler flags for cross-compilation:

- `-target x86_64-elf` - Clang's built-in cross-compile to bare metal x86_64
- `-ffreestanding` - No hosted environment (no OS)
- `-nostdlib` - No standard library linking
- `-mno-red-zone` - Disable red zone (x86_64 ABI quirk for kernel code)
- `-fno-stack-protector` - No stack canaries (we're the OS!)

[!side]
The "red zone", despite it's weird name, does not refer to a dangerous area or the üöace of high stakes in a football game. 
In the x86_64 calling convention, it is a 128-byte area below the stack pointer that functions can use without adjusting `rsp`. Interrupts break this assumption in kernel code.
[/!side]

Don't worry about memorizing these. We'll explain each flag when we use it.

---

**Next: [Chapter 3: The Boot Process](../chapter-03-boot-process/README.md)**
