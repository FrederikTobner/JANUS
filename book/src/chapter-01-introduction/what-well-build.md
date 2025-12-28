# What We'll Build

**TinyOS** is a minimalist operating system for x86_64 architecture. By the end of this book, it will:

- Boot on real hardware (or a virtual machine)  
- Support 64-bit long mode  

> **TODO:** Add other features once we have implemented them

[!side]
We're building a **teaching OS**—clarity over performance. Linux took 30 years and thousands of contributors to reach its current state. We're not competing with that.
[/!side]

What it *won't* do: network stack, advanced scheduling, POSIX compliance, or graphics. Text mode is enough to learn OS concepts.

> TODO: Consider modeling OS after DOS to some extend (simple, single-user, single-task, and direct hardware access) in the first chapters and then transition to UNIX. First we ignore multitasking, user-space, memory protection, etc.

TinyOS develops incrementally, with each chapter building on the previous:

[!side]
Each chapter follows the same pattern: concepts first, then incremental implementation with working code at each step. We show failures and debugging, not just successes.
[/!side]

1. **Introduction** - What we're building and why (you are here)
2. **Getting Started** - Toolchain setup and verification
3. **Boot Process** - From power-on to running C code
4. **Input/Output** - Serial debugging, screen output, keyboard input
5. **Memory Management** - Physical and virtual memory
6. **Kernel Basics** - Interrupts, system calls, and user programs

---

**Next: [Prerequisites](prerequisites.md)**
