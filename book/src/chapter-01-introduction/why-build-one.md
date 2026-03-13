# Why Build One?

Legitimate question: Linux exists. Windows exists. macOS exists. Why spend hundreds of hours building your own operating system, which will never run your browser, never support your GPU, and will probably crash when you look at it the wrong way?

Because reading about operating systems is one thing; debugging a kernel that keeps triple-faulting is another. You'll understand memory management at a level that reading Wikipedia never achieves.

[!side]
**Triple Fault**: When the CPU encounters an error, it triggers a fault. If the *fault handler* faults, that's a double fault. If the *double fault handler* faults, the CPU gives up and immediately resets—instant reboot with no error message. For OS developers, it's the bane of existence. Most common during boot when setting up page tables.
[/!side]

It's like the difference between reading how to swim and jumping into the deep end of the pool. And yes, there are actual books about swimming—legend has it that Norbert Wiener, a mathematician, learned how to swim by reading one.
Theory teaches you concepts, while practice makes the concepts stick—sometimes forcefully.

In application programming, you're multiple abstraction layers away from the hardware. When working on your own operating system, you write directly to memory-mapped I/O and device registers.

[!side]
When a bug causes mysterious reboots, the lack of a runtime and OS services means you have to figure out what's going wrong.
A hosted OS would normally deliver a signal (or similar exception) when you dereference invalid memory. In a kernel, there’s no supervisor to rescue you.
Additionally, in user space we could often generate a core dump and analyze it later, but here we won't have that luxury.
[/!side]

---

**Next: [What We'll Build](what-well-build.md)**
