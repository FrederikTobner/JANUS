# What We'll Build

**TinyOS** is a minimalist operating system for x86_64 architecture. By the end of this book, it will:

- Boot on real hardware (or a virtual machine)  
- Support 64-bit long mode  

> **TODO:** Add other features once we have implemented them

[!side]
We're building a **teaching OS**, focusing on simplicity and clarity over performance or features.
[/!side]

> TODO: Make sure other stuff that we haven't implemented is mentioned here as well, after we finished the book 

I won't implement a network stack, advanced scheduling algorithms, or POSIX compliance. The goal is to understand core OS concepts, not build a production-ready system.

> TODO: Consider modeling OS after DOS to some extend (simple, single-user, single-task, and direct hardware access) in the first chapters and then transition to UNIX. First we ignore multitasking, user-space, memory protection, etc.

The development will be done incrementally, with each chapter building on the previous:

---

**Next: [Prerequisites](prerequisites.md)**
