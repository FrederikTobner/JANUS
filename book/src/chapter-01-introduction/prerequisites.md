# Prerequisites

Before diving into OS development, you need a solid foundation in several areas. This isn't application programming where the OS handles the hard parts.
We are building the OS itself, so you should already be comfortable with a few foundational topics.

If any of these feel unfamiliar, get comfortable with them first.
Writing your own operating system is widely considered one of the most challenging programming projects. For that reason, it helps to start from a solid foundation.

On the programming side, you should be very comfortable with **C programming** and have a basic understanding of **assembly language**. You don't need to be an expert assembly programmer, but you should understand how high-level constructs translate to low-level operations.

[!side]
Operating system development is where C truly shines. No garbage collection, direct memory access, and plenty of inline assembly. For better or worse, this is C's home turf.
[/!side]

As our build system, we will use **CMake**. Since explaining CMake from scratch would extend the scope of the book quite a bit, you should be familiar with the basics.

Finally, a basic understanding of **computer architecture** is necessary. You don't need to know all the ins and outs of CPU design, but you should understand how a CPU executes instructions, how memory works, and how function calls are handled at a low level.

You don't need prior knowledge of OS internals; we'll build that understanding together.
The x86_64 architecture is the target architecture for our OS, but you don't need to know its specifics beforehand. We will cover everything necessary as we go along.

We'll cover what you need, when you need it.

Additionally, prior knowledge about bootloaders or LLVM/Clang internals is not required.

Before we start, ask yourself the following questions:

- Are you comfortable writing and debugging C (including pointers and bit-level operations)?
- Can you read basic x86_64 assembly and follow what a snippet does?
- Do you understand the basics of how CPUs execute instructions and how memory is addressed?
- Are you comfortable using a build system (CMake) and a command-line toolchain?

If you need to get familiar with these topics, here are resources that can be used to provide the necessary background knowledge.
These may not be the absolute best resources out there, but they are what I have found useful in the past.

For **C programming**, both "Learn C Programming" by Jeff Szuhay and "The C Programming Language" by Kernighan & Ritchie can act as good starting points if you have never programmed in C.
If you are already familiar with the basics, "Extreme C" by Kamran Amini or "Pointers in C Programming" by Thomas Mailund can help deepen your understanding.

For learning **assembly**, "Programming Boot Sector Games" by Oscar Toledo G. is a good start, but it only covers 8086/8088 assembly.

> TODO: Find and read more resources for assembly programming, preferably x86_64 assembly, so we can vouch for them

> TODO: Find and read more resources for basic computer architecture and cmake, so we can vouch for them

We develop on **Linux** because it offers the smoothest OS-development workflow with the least friction.
If you want, you can also try to use Windows or macOS, but since we are using GNU tools and parts of the LLVM toolchain, Linux is the path of least resistance.

[!side]
ARM would be tempting as well but its not as common on desktops, making it less likely that readers have access to hardware to test on.
[/!side]

TinyOS targets the **x86_64** architecture. It is ideal for learning OS development because it is widely used and well-documented. Additionally, most developers have access to x86_64 hardware, which is crucial for testing and running our operating system on real hardware.
The rich ecosystem of tools and documentation further supports development on this architecture.

[!side]
The “editor wars” refer to the long-running rivalry between users of the console-based editors Vi and Emacs. Since 1985, there have been numerous debates between the communities of both editors, each claiming that their choice is the paragon of editing perfection.
[/!side]

Use whatever editor you prefer.
The code we will write will be the same regardless.
I won't participate in the editor wars in this book.

---

**Next: [How to use this book](how-to-use.md)**
