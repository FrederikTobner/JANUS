# JANUS Concepts

This wiki covers the foundational concepts that underpin operating system development. The topics presented here are general-purpose — they are not specific to JANUS or any other particular kernel — and a reader with a reasonable grasp of C programming should be able to follow along without prior OS experience.

Each article aims to explain a single concept thoroughly enough that the reader can understand how it works and why it matters, without descending into the minutiae of a textbook. Where a concept has a direct application in JANUS, a margin note links to the corresponding page in the [Reference](../reference/introduction.md) section.

The articles are organised by topic area:

- **Hardware** covers the physical devices and interfaces an OS must interact with — framebuffers, serial ports, memory-mapped I/O.
- **Boot** explains the sequence of events between pressing the power button and executing the first line of kernel code.
- **Memory** describes how physical RAM is managed, how virtual address spaces work, and how page tables translate between the two.
- **Tools** provides practical cheat sheets for developer tooling that may be unfamiliar even to experienced C programmers, including LLDB, CMake, and QEMU.
