# Why Build One?

Legitimate question: Linux exists. Windows exists. macOS exists. Why spend hundreds of hours building your own operating system, which will never run your browser, never support your GPU, and probably just crash hwn you look at it in the wrong way?

Because:


Reading about paging is one thing. Debugging why your page table entries keep triple-faulting the CPU is *entirely different*. You'll understand memory management at a level that reading Wikipedia never achieves.

[!side]
**Triple Fault**: When the CPU encounters an error, it triggers a fault. If the *fault handler* faults, that's a double fault. If the *double fault handler* faults, the CPU gives up and immediately resets—instant reboot with no error message. For OS developers, it's the bane of existence. Most common during boot when setting up page tables.
[/!side]

It's like the difference between reading a recipe and actually cooking. Theory teaches you concepts. Practice beats them into your skull with a hammer made of segmentation faults.


Operating systems seem magical: you press the power button and somehow a pixel-perfect interface appears. By building one, you'll see that there's no magic—just layers of well-crafted abstractions.


In application programming, you're 47 abstraction layers away from the hardware. Here? You're writing directly to memory-mapped I/O ports. You're *teaching the CPU how to handle page faults*.

[!side]
When a bug causes mysterious reboots, the lack of a runtime and OS services means *you* have to figure out what's going wrong. No stack traces, no logging frameworks—just you and your wits.
[/!side] 

OS development teaches you:

- Low-level programming (C, assembly)
- Hardware architecture (CPU modes, memory mapping, interrupts)
- Debugging skills (when your debugger crashes, what do you do?)
- Systems thinking (how components interact)

These skills apply far beyond OS development.

---

**Next: [What We'll Build](what-well-build.md)**
