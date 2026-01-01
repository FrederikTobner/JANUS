# Why Build One?

Legitimate question: Linux exists. Windows exists. macOS exists. Why spend hundreds of hours building your own operating system, which will never run your browser, never support your GPU, and probably just crash when you look at it in the wrong way?

Becaue reading about what constitutes an operating system is one thing. Debugging why kernel keeps triple-faulting is *entirely different*. You'll understand memory management at a level that reading Wikipedia never achieves.

[!side]
**Triple Fault**: When the CPU encounters an error, it triggers a fault. If the *fault handler* faults, that's a double fault. If the *double fault handler* faults, the CPU gives up and immediately resets—instant reboot with no error message. For OS developers, it's the bane of existence. Most common during boot when setting up page tables.
[/!side]

It's like the difference between reading a recipe and actually cooking. Theory teaches you concepts. Practice beats them into your skull with a hammer made of segmentation faults.

In application programming, you're 47 abstraction layers away from the hardware. Here? You're writing directly to memory-mapped I/O ports. You're *teaching the CPU how to handle page faults*.

[!side]
When a bug causes mysterious reboots, the lack of a runtime and OS services means *you* have to figure out what's going wrong. The operaing system would usually send a singal to the process notifying it that a segmentation fault occurred, but here there is no OS to do that for you. Additionally in user space we would be able to just generate a core dump and analyze it later, but in OS development we don't have that luxury
[/!side] 

---

**Next: [What We'll Build](what-well-build.md)**
