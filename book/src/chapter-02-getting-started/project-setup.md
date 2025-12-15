# Project Setup

Now that your tools are installed and verified, let's create the project structure.

[!side]
We build incrementally. Each chapter adds structure as we need it, not all upfront.
[/!side]

## Creating the Project Directory

```bash
mkdir -p TinyOS
cd TinyOS

# Initialize git repository
git init
```

## Initial Directory Structure

We'll build the project incrementally throughout the book. For now, just create the top-level directory. Each chapter adds new components:

- **Chapter 2**: Boot code, build system, Multiboot header
- **Chapter 3**: I/O drivers (serial, VGA, keyboard)
- **Chapter 4**: Memory management (physical and virtual)
- **Chapter 5**: Kernel features (interrupts, system calls)

Don't create subdirectories yet—we'll build them piece by piece as we understand what we're building.

[!side]
Premature structure leads to `misc/`, `utils/`, and `stuff/` directories. We avoid that by building only what we need.
[/!side]

## What We're Not Covering

This chapter focuses on *toolchain setup*, not:

- **Editor configuration**: Use whatever you prefer. The project includes `.clangd` for LSP-compatible editors (Language Server Protocol - provides autocomplete, go-to-definition, error highlighting) if you want it.

[!side]
Vim, Emacs, VS Code, whatever. The book doesn't assume an editor. If you want IDE features, clangd works with most editors.
[/!side]

- **Git workflows**: Assumed knowledge.
- **Shell basics**: You should already know how to navigate directories and run commands.
- **C language tutorial**: We'll write plenty of C, but won't explain pointers or struct syntax.
- **CMake tutorials**: We'll use it extensively, but won't teach the basics.

We're here to build an OS, not learn prerequisite tools. If those feel unfamiliar, get comfortable with them first.

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

## Next Steps

You now have a complete TinyOS development environment and an empty project directory.

In the next chapter, we'll:

1. Create the project structure
2. Write a Multiboot2 header for GRUB
3. Write boot assembly code
4. Set up the build system with CMake
5. Create a bootable ISO
6. Boot our kernel in QEMU!

---

**Next: [Chapter 2: The Boot Process](../chapter-02-boot-process/README.md)**
