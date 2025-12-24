# Project Setup

Now that your tools are installed and verified, let's create the project structure.

[!side]
We build incrementally. Each chapter adds structure as we need it, not all upfront.
[/!side]

```bash
mkdir -p TinyOS
cd TinyOS
```

We'll build the project incrementally throughout the book. For now, just create the top-level directory. Each chapter adds new components as we need them.

> TODO: This is probably outdated

```mermaid
graph TD
    CH2["<b>Chapter 3: Boot Process</b><br/>TinyOS/<br/>├── boot/<br/>├── kernel/<br/>├── cmake/<br/>└── CMakeLists.txt"]
    CH3["<b>Chapter 4: I/O</b><br/>TinyOS/<br/>├── boot/<br/>├── kernel/<br/>├── drivers/<br/>├── cmake/<br/>└── CMakeLists.txt"]
    CH4["<b>Chapter 5: Memory</b><br/>TinyOS/<br/>├── boot/<br/>├── kernel/<br/>├── drivers/<br/>├── mm/<br/>├── cmake/<br/>└── CMakeLists.txt"]
    CH5["<b>Chapter 6+: Full Kernel</b><br/>TinyOS/<br/>├── boot/<br/>├── kernel/<br/>├── drivers/<br/>├── mm/<br/>├── lib/<br/>├── arch/<br/>├── include/<br/>├── cmake/<br/>└── CMakeLists.txt"]
    
    CH2 --> CH3
    CH3 --> CH4
    CH4 --> CH5
```

**Chapter-by-chapter additions:**

- **Chapter 3**: Boot code, build system, Multiboot header
- **Chapter 4**: I/O drivers (serial, VGA, keyboard)
- **Chapter 5**: Memory management (physical and virtual)
- **Chapter 6**: Kernel features (interrupts, system calls)

Don't create subdirectories yet—we'll build them piece by piece as we understand what we're building.

[!side]
Premature structure leads to `misc/`, `utils/`, and `stuff/` directories. We avoid that by building only what we need.
[/!side]

## Cross-Compilation Flags

Throughout this book, we'll use these compiler flags for cross-compilation:

- `-target x86_64-elf` - Clang's built-in cross-compile to bare metal x86_64
- `-ffreestanding` - No hosted environment (no OS)
- `-nostdlib` - No standard library linking
- `-mno-red-zone` - Disable red zone (x86_64 ABI quirk for kernel code)
- `-fno-stack-protector` - No stack canaries (we're the OS!)

[!side]
Red zone is a 128-byte area below the stack pointer that functions can use without adjusting `rsp`. Interrupts break this assumption in kernel code.
[/!side]

Don't worry about memorizing these. We'll explain each flag when we use it.

---

**Next: [Chapter 2: The Boot Process](../chapter-03-boot-process/README.md)**
