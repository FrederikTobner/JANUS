# I/O

This section covers the two mechanisms a kernel uses to communicate with hardware devices: port-mapped I/O and memory-mapped I/O. Serial ports are treated in depth because they are typically the first output channel a kernel brings up, well before any display driver is available.

**Prerequisites:** [Memory](../memory/memory.md). MMIO regions are mapped into the virtual address space, so understanding page tables and virtual memory is necessary before reasoning about how device registers are accessed.

## Articles

Read in order.

1. **[Serial Ports](serial-ports.md)** — The UART hardware, baud rate configuration, why serial output is the first thing most kernel developers bring up, and the differences between x86 port-mapped COM ports and the aarch64 PL011.
2. **[Memory-Mapped I/O](mmio.md)** — Port-mapped vs memory-mapped I/O, `volatile`-qualified access, and the requirement to mark MMIO regions as uncacheable in the page tables.
