# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts—we're building the OS itself.

## Required Knowledge

If any of these feel unfamiliar, get comfortable with them first. The OS development is challenging enough on its own.

**Programming Skills:**

- **C language**: Pointers, structs, memory management, function pointers, bitwise operations
- **Assembly basics**: Registers, instruction execution, calling conventions, stack operations

[!side]
Operating system development is where C truly shines. No garbage collection, direct memory access and a lot of inline assembly. For better or worse this is C's home turf.
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

If you need to get familiar with these topics, here are resources that can

For **C Programming**  both "Learn C Programming" by Jeff Szuhay and "C Programming Language" by Kernighan & Ritchie can act as a good starting point if you have never programmed in C. If you are already familiar with the basics "Extreme C" by Kamran Amini, can help deepen your understanding.

For learning **Assembly** "Programming boot sector games" by Oscar Toledo G. is a good start, but it only covers 8086/8088 assembly.

> TODO: Find resources for basic computer architecture and cmake

We develop on **Linux**. TinyOS targets **x86_64** architecture.

**Why Linux?**

- Native toolchain support
- Direct access to development tools
- No virtualization overhead for testing
- Industry-standard OS development environment

[!side]
WSL2 on Windows works as well, to build the iso. To run it will be easier to install QEMU under Windows directly.
macOS also works but requires Homebrew for GRUB tools.
[/!side]

**Why x86_64?**

The x86_64 architecture is ideal for learning OS development because it is widely used and well-documented. Additionally most developers have access to x86_64 hardware, making it easier to test and run the OS. The rich ecosystem of tools and documentation further supports development on this architecture. Finally QEMU provides excellent emulation for x86_64, allowing for easy testing and debugging.

[!side]
ARM would be tempting as well but its not as common on desktops, making it less likely that readers have access to hardware to test on.
[/!side]

**Your editor**: Use whatever you prefer. Vim, Emacs, VS Code, CLion, Notepadwhatever makes you productive. The code we will write will be the same regardless.

---

**Next: [Common Misconceptions](misconceptions.md)**
