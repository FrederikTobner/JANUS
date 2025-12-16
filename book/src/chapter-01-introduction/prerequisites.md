# Prerequisites

This book assumes you're already a competent programmer with:

- **C language**: Pointers, structs, function pointers, bitwise operations, manual memory management
- **Assembly basics**: Registers, instruction execution, calling conventions
- **Build tools**: CMake, command line, Git workflows

## Computer Architecture

- **CPU fundamentals**: Registers, instruction execution, stack operations
- **Memory concepts**: RAM, addresses, virtual vs. physical memory
- **Number systems**: Binary, hexadecimal, bitwise operations

[!side]We'll explain x86_64 specifics as needed (long mode, page tables, etc.), but you should understand general CPU/memory architecture.[/!side]

## Self-Assessment: Are You Ready?

Before diving in, make sure you can comfortably answer these:

**C Programming:**  
Implement a linked list with manual memory management • Explain `char *p` vs. `char **p` • What happens calling `free()` twice on the same pointer? • `static` at file scope vs. function scope?

**Assembly & Architecture:**  
Purpose of stack pointer • Function calls at assembly level • Physical vs. virtual addresses • How CPU tracks next instruction?

**Systems Concepts:**  
What happens when you type `ls` and press Enter? • How do programs access files? • Process vs. thread?

[!side]If these feel too basic, you're ready. If they're unclear, study fundamentals first. You can't learn OS development and C simultaneously—it's like learning to juggle while riding a unicycle.[/!side]

If some feel unclear, **spend time with foundational resources first**. This book won't teach the basics—it builds on them.

You **don't** need prior experience with:

- Operating system internals (that's what we're here to learn)
- x86_64 architecture specifics (we'll cover what's needed)
- Bootloaders or low-level hardware (we build from scratch)
- LLVM/Clang internals (basic usage is sufficient)

---

**Next: [Common Misconceptions](misconceptions.md)**
