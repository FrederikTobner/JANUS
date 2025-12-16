# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts—we're building the OS itself.

[!side]
If you can write a linked list in C and understand what `mov eax, 5` does, you're ready.
[/!side]

## Required Knowledge

### Programming Skills

- **C language**: Pointers, structs, memory management, function pointers, bitwise operations
- **Assembly basics**: Registers, instruction execution, calling conventions, stack operations

[!side]
OS development is where C truly shines. No garbage collection, direct memory access, inline assembly—this is C's home turf.
[/!side]

### Build Systems

- **CMake**: Creating targets, linking libraries, setting compiler flags, subdirectories
- **Make/Ninja**: Basic understanding of build automation
- **Command line**: Shell navigation, basic unix commands

If any of these feel unfamiliar, get comfortable with them first. The OS development is challenging enough without fighting your tools.

## Computer Architecture Fundamentals

You should understand:

- **CPU operation**: How registers work, instruction execution, the fetch-decode-execute cycle
- **Memory concepts**: RAM, addresses, the difference between virtual and physical memory
- **Number systems**: Binary, hexadecimal, two's complement, bitwise operations
- **The stack**: How function calls work, stack frames, return addresses

## What You DON'T Need

You don't need prior experience with:

- Operating system internals (that's what we're here to learn)
- x86_64 architecture specifics (we'll cover what's needed)
- Bootloaders or low-level hardware (we build from scratch)
- LLVM/Clang internals (basic usage is sufficient)

## Self-Assessment

Ask yourself:

**Can you explain what this C code does?**

**Can you read this assembly?**

```asm
mov eax, 5
add eax, 3
```

**Do you understand this?**

```c
struct node {
    int data;
    struct node *next;
};
```

If any of these feel confusing, spend time with foundational resources first. OS development is challenging enough without simultaneously learning prerequisite skills.

## Recommended Background Reading

If you need to brush up:

- **C Programming**: "The C Programming Language" by Kernighan & Ritchie
- **Assembly**: "Programming from the Ground Up" by Jonathan Bartlett
- **Computer Architecture**: "Computer Systems: A Programmer's Perspective" by Bryant & O'Hallaron

## Development Environment

We develop on **Linux** (or WSL2 on Windows). TinyOS targets **x86_64** architecture.

**Why Linux?**

- Native toolchain support
- Direct access to development tools
- No virtualization overhead for testing
- Industry-standard OS development environment

[!side]
WSL2 on Windows works great for this. macOS also works but requires Homebrew for GRUB tools.
[/!side]

**Why x86_64?**

- Widely documented architecture
- QEMU has excellent x86_64 emulation
- Most developers have access to x86_64 hardware
- Rich ecosystem of tools and documentation

[!side]
ARM is tempting but its documentation is scattered across vendor-specific implementations. x86_64 has the Intel and AMD manuals—everything you need.
[/!side]

- **Your editor**: Use whatever you prefer. The project includes `.clangd` for LSP-compatible editors (autocomplete, go-to-definition, error highlighting) if you want it.

[!side]
Vim, Emacs, VS Code, CLion—whatever makes you productive. The code is the same regardless.
[/!side]

## Time Commitment

OS development is not a weekend project. Expect:

- **Chapter 2-3**: 2-4 hours (setup and first boot)
- **Chapter 4**: 4-6 hours (memory management)
- **Chapter 5**: 6-8 hours (I/O and drivers)
- **Chapter 6+**: 8-12 hours each (kernel features)

Take your time. Understanding is more valuable than speed.

---

**Next: [Common Misconceptions](misconceptions.md)**
