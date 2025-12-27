# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts—we're building the OS itself.

## Required Knowledge

If any of these feel unfamiliar, get comfortable with them first. The OS development is challenging enough on its own.

**Programming Skills:**

- **C language**: Pointers, structs, memory management, function pointers, bitwise operations
- **Assembly basics**: Registers, instruction execution, calling conventions, stack operations

[!side]
OS development is where C truly shines. No garbage collection, direct memory access, inline assembly—this is C's home turf.
[/!side]

**Build Systems:**

- **CMake**: Creating targets, linking libraries, setting compiler flags, subdirectories
- **Command line**: Shell navigation, basic unix commands

**Computer Architecture Fundamentals:**

- **CPU operation**: How registers work, instruction execution, the fetch-decode-execute cycle
- **Memory concepts**: RAM, addresses, the difference between virtual and physical memory
- **Number systems**: Binary, hexadecimal, two's complement, bitwise operations
- **The stack**: How function calls work, stack frames, return addresses

**You don't need to know:**

- Operating system internals (that's what we're here to learn)
- x86_64 architecture specifics (we'll cover what's needed)
- Bootloaders or low-level hardware (we build from scratch)
- LLVM/Clang internals (basic usage is sufficient)

## Self-Assessment

Ask yourself:


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
void main() {
    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = 42;
    new_node->next = NULL;
}
```

and do you know how static keyword behaves when used inside a function vs globally?

```c
static int counter_global = 0;
void increment() {
    counter_global++;
    static int counter_local = conter_global;
    counter_global++;
}
void main() {
    increment();
    increment();
}
```

If any of these feel confusing, spend time with foundational resources first. OS development is challenging enough without simultaneously learning prerequisite skills.

## Recommended Background Reading

If you need to brush up:

- **C Programming**: "Learn C Programming" by Jeff Szuhay, "C Programming Language" by Kernighan & Ritchie and "Extreme C" by Kamran Amini
- **Assembly**: "Programming from the Ground Up" by Jonathan Bartlett
- **Computer Architecture**: "Computer Systems: A Programmer's Perspective" by Bryant & O'Hallaron

## Development Environment

We develop on **Linux**. TinyOS targets **x86_64** architecture.

**Why Linux?**

- Native toolchain support
- Direct access to development tools
- No virtualization overhead for testing
- Industry-standard OS development environment

[!side]
WSL2 on Windows works as well, to build the iso we will boot. To run it will be easier to install QEMU under Windows directly.
macOS also works but requires Homebrew for GRUB tools.
[/!side]

**Why x86_64?**

The x86_64 architecture is ideal for learning OS development because it is widely used and well-documented. Additionally most developers have access to x86_64 hardware, making it easier to test and run the OS. The rich ecosystem of tools and documentation further supports development on this architecture. Finally QEMU provides excellent emulation for x86_64, allowing for easy testing and debugging.

[!side]
ARM would be tempting as well but its documentation is scattered across vendor-specific implementations. x86_64 has the Intel and AMD manuals—everything you need.
[/!side]

- **Your editor**: Use whatever you prefer. The project includes `.clangd` for LSP-compatible editors (autocomplete, go-to-definition, error highlighting) if you want it.

[!side]
Vim, Emacs, VS Code, CLion—whatever makes you productive. The code is the same regardless.
[/!side]


---

**Next: [Common Misconceptions](misconceptions.md)**
