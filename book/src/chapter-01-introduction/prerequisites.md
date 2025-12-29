# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts—we're building the OS itself.

## Required Knowledge

If any of these feel unfamiliar, get comfortable with them first. Writing your own Operating system is usually considered one of the most challenging programming task there is, therfore it is crucial that you have already a solid foundation that we can build upon:

On the programming side, you should be very comfortable with C programming and have a basic understanding of assembly language. You don't need to be an expert assembler programmer, but you should understand how high-level constructs translate to low-level operations.

[!side]
Operating system development is where C truly shines. No garbage collection, direct memory access and a lot of inline assembly. For better or worse this is C's home turf.
[/!side]

As our build system we will use CMake. Since explaining CMake from scratch would extend the scope of the book quite a bit, you should be familiar with the basics of CMake and command line usage.

Finally a basic understanding of computer architecture is necessary. You don't need to know all the ins and outs of CPU design, but you should understand how a CPU executes instructions, how memory works, and how function calls are handled at a low level.

There is no need to have any prior knowledge about operating system internals and design, as that is what we will be learning together in this book.
The x86_64 architecture is the target architecture for our OS, but you don't need to know its specifics beforehand. We will cover everything necessary as we go along. Additionally any prior knowledge about bootloaders, or LLVM/Clang internals is not required.

So ask yourself:

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

If you need to get familiar with these topics, here are resources that can be used to provide the necessary background knowledge.
These may not be the absolute best resources out there, but they are what I have found useful in the past.

For **C Programming**  both "Learn C Programming" by Jeff Szuhay and "C Programming Language" by Kernighan & Ritchie can act as a good starting point if you have never programmed in C.
If you are already familiar with the basics "Extreme C" by Kamran Amini or "Pointers in C Programming" from Thomas Mailund, can help deepen your understanding.

For learning **Assembly** "Programming boot sector games" by Oscar Toledo G. is a good start, but it only covers 8086/8088 assembly.

> TODO: Find resources for basic computer architecture and cmake

We develop on **Linux** becuase it provides the best environment for OS development without much tinkering.
If you want you can also try to use Windows or MacOs, but since we are using GNU tools and parts of the LLVM toolchain, Linux is the path of least resistance.

[!side]
ARM would be tempting as well but its not as common on desktops, making it less likely that readers have access to hardware to test on.
[/!side]

TinyOS targets **x86_64** architecture. It is ideal for learning OS development because it is widely used and well-documented. Additionally most developers have access to x86_64 hardware, which is crucial to being able to test our operating system on real hardware.
The rich ecosystem of tools and documentation further supports development on this architecture. Finally QEMU provides excellent emulation for x86_64, allowing for easy testing and debugging.

[!side]
The “editor wars” refer to the long-running rivalry between users of the console based editors Vi and Emacs. Since 1985 there have been 
numerous debates have occured between the communities of both editors, each claiming that their choice is the paragon of editing perfection.
[/!side]

As your editor you should use whatever you prefer. 
The code we will write will be the same regardless. I won't participate in the editor wars here.

---

**Next: [Development Environment](development-environment.md)**

