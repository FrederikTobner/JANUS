# Chapter 2: Getting Started

> *"Give me six hours to chop down a tree and I will spend the first four sharpening the axe."*  
> — Abraham Lincoln

Before we can build an operating system, we need to set up our development environment. This isn't glamorous work, but it's essential. OS development requires specialized tools that differ from typical application development.
[!side]
"Bare metal" means code that runs directly on hardware with no OS layer. Your first instruction executes in 16-bit real mode!
[/!side]
Unlike normal programming where you have an operating system beneath you, we're building the OS itself. There's no standard library to link against, no runtime to handle initialization. We will be running on bare metal. 

## What You'll Learn

In this chapter, you'll set up:

- Cross-compilation toolchain (Clang or GCC)
- Build system (CMake and Ninja)
- Assembly tools (NASM)
- Debugging and testing environment (QEMU, LLDB)
- Bootloader tools (GRUB, xorriso, mtools)

---

**Next: [Prerequisites](prerequisites.md)**
