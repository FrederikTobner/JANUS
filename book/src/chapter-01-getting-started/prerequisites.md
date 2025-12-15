# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts—we're building the OS itself.

## Required Knowledge

### Programming Skills

- **C language**: Pointers, structs, memory management, function pointers, bitwise operations
- **Assembly basics**: Registers, instruction execution, calling conventions, stack operations
- **Command line**: Shell navigation, build tools, environment variables

### Build Systems

- **CMake**: Creating targets, linking libraries, setting compiler flags, subdirectories
- **Make/Ninja**: Basic understanding of build automation

### Version Control

- **Git**: Branching, committing, checking out code, basic workflows

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

```c
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
```

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

**Why x86_64?**

- Widely documented architecture
- QEMU has excellent x86_64 emulation
- Most developers have access to x86_64 hardware
- Rich ecosystem of tools and documentation

## Time Commitment

OS development is not a weekend project. Expect:

- **Chapter 1-2**: 2-4 hours (setup and first boot)
- **Chapter 3**: 4-6 hours (memory management)
- **Chapter 4**: 6-8 hours (I/O and drivers)
- **Chapter 5+**: 8-12 hours each (kernel features)

Take your time. Understanding is more valuable than speed.

---

**Next: [Installing Tools](installing-tools.md)**
