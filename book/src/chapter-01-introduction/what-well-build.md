# What We'll Build

**TinyOS** is a minimalist operating system for x86_64 architecture. By the end of this book, it will:

- Boot on real hardware (or a virtual machine)  
- Support 64-bit long mode  
- Manage physical and virtual memory  
- Handle interrupts and exceptions  
- Run user-space programs  
- Provide basic device drivers (keyboard, display)  

[!side]We're building a **teaching OS**—clarity over performance. Linux took 30 years and thousands of contributors to reach its current state. We're not competing with that.[/!side]

What it *won't* do: network stack, advanced scheduling, POSIX compliance, or graphics. Text mode is enough to learn OS concepts.

## What You'll Learn

TinyOS develops incrementally, with each chapter building on the previous:

1. **Introduction** - What we're building and why (you are here)
2. **Getting Started** - Toolchain setup and verification
3. **Boot Process** - From power-on to running C code
4. **Input/Output** - Serial debugging, screen output, keyboard input
5. **Memory Management** - Physical and virtual memory
6. **Kernel Basics** - Interrupts, system calls, and user programs

[!side]Each chapter follows the same pattern: concepts first, then incremental implementation with working code at each step. We show failures and debugging, not just successes.[/!side]

The hardest conceptual jump is the boot process (Chapter 3), where you'll learn how the CPU actually works. After that, everything builds on established patterns.

---

**Next: [Prerequisites](prerequisites.md)**
