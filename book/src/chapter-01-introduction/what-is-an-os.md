# What is an Operating System?

Ask a programmer what an OS does and you'll get: "It... runs programs?" Which is true but unhelpful, like saying a car "moves you places."

At its core, an operating system is a **resource manager**—a bossy middle manager between your programs and the hardware, coordinating access to CPU time, memory, I/O devices, and storage.

[!side]Modern OSes manage thousands of processes, gigabytes of memory, and dozens of devices simultaneously. All while maintaining the illusion that each program has the machine to itself.[/!side]

But an OS is more than just a manager—it's also an **abstraction layer**. It hides the messy details of hardware from applications, providing clean interfaces like:

```c
// Instead of programming disk controllers directly:
FILE *f = fopen("data.txt", "r");

// Instead of managing physical memory addresses:
void *ptr = malloc(1024);

// Instead of context switching manually:
pid_t pid = fork();
```

[!side]The same `fopen()` call works whether you're on Linux with ext4, Windows with NTFS, or macOS with APFS. That's the power of abstraction.[/!side]

These abstractions let programmers write portable software without worrying about the specific hardware underneath.

---

**Next: [Why Build One?](why-build-one.md)**
