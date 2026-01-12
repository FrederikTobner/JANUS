# What is an Operating System?

Ask a programmer what an OS does and you'll get: "Well... it runs programs?" 
Which is true but not really helpful, if you plan to implement it yourself or just look under the hood and get a glimpse of the different components work together to make this happen.
At its core, an operating system is a **resource manager**.
You can think of it as a bossy middle man between your programs and the hardware resources available on the current system, coordinating access to CPU time, memory, I/O devices, storage devices, etc. 

[!side]
Modern operating systems manage thousands of processes, gigabytes of memory, and dozens of devices simultaneously. 
All while maintaining the illusion that each program has the machine to itself.
[/!side]

But an OS is more than just a manager.
It's also acts as an **abstraction layer**, that hides the messy details of hardware from applications, providing an interface that can be used by developers working in the user space.

```c
// Instead of programming disk controllers directly:
FILE *f = fopen("data.txt", "r");

// Instead of managing physical memory addresses:
void *ptr = malloc(1024);

// Instead of context switching manually:
pid_t pid = fork();
```

[!side]
The same `fopen()` call works whether you're on Linux with ext4, Windows with NTFS, or macOS with APFS. That's the power of abstraction.
[/!side]

These abstractions also let programmers write portable software without worrying about the specific hardware and file system underneath.

---

**Next: [Why Build One?](why-build-one.md)**
