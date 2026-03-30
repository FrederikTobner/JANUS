# JANUS Wiki

This wiki covers the foundational concepts that underpin operating system development. The topics presented here are general-purpose — they are not specific to JANUS or any other particular kernel — and a reader with a reasonable grasp of C programming should be able to follow along without prior OS experience.

Each article aims to explain a single concept thoroughly enough that the reader can understand how it works and why it matters, without descending into the minutiae of a textbook. Where a concept has a direct application in JANUS, a side note links to the corresponding page in the [Project](../project/introduction.md) section.

The articles are organised by topic area:

- **Boot** explains the sequence of events between pressing the power button and executing the first line of kernel code.
- **Memory** describes how physical RAM is managed, how virtual address spaces work, and how page tables translate between the two.
- **I/O** covers the interfaces an OS must use to interact with hardware — serial ports and memory-mapped I/O.
- **Graphics** covers display hardware — VGA text mode and framebuffers.
