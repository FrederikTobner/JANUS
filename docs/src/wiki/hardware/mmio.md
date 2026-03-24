# Memory-Mapped I/O

There are two fundamental mechanisms by which a CPU communicates with hardware devices: port-mapped I/O (PMIO) and memory-mapped I/O (MMIO). Both allow the processor to read from and write to device registers, but they differ in how those registers are addressed and accessed.

## Port-Mapped I/O

In port-mapped I/O, the processor has a separate address space dedicated to I/O devices, distinct from the memory address space used for RAM. Access to this I/O space requires special instructions — on x86, these are `in` and `out`. Each device is assigned a range of port numbers, and the CPU uses these instructions to transfer data one byte, word, or doubleword at a time.

Port-mapped I/O is a legacy of the x86 architecture and is used by classic PC peripherals: the 16550 UART (COM1 at port `0x3F8`), the 8259 PIC, the PS/2 keyboard controller, and the VGA subsystem. It does not exist on ARM or RISC-V architectures.

## Memory-Mapped I/O

In memory-mapped I/O, device registers are mapped into the same address space as ordinary memory. The CPU accesses them using regular load and store instructions — there are no special I/O instructions required. A region of the physical address space is reserved for the device, and writes to addresses in that region go to the device's registers rather than to RAM.

MMIO is the dominant I/O model on modern hardware and is the only option on architectures like aarch64. PCIe devices, modern UARTs (such as the PL011), framebuffers, and most SoC peripherals are all memory-mapped.

## Volatile Access

Because the compiler has no way of knowing that a memory address refers to a hardware register rather than ordinary RAM, it may optimise away reads and writes that it considers redundant. To prevent this, all MMIO accesses must go through `volatile`-qualified pointers, which instruct the compiler to emit every load and store exactly as written and in the order specified.

A common pattern in kernel code is a pair of helper functions:

```c
static inline u32 mmio_read32(void volatile *addr) {
    return *(u32 volatile *)addr;
}

static inline void mmio_write32(void volatile *addr, u32 value) {
    *(u32 volatile *)addr = value;
}
```

These wrappers centralise the `volatile` semantics and make MMIO accesses visually distinct from ordinary memory operations.

## Cacheability

MMIO regions must not be cached. If the CPU caches a write to a device register, the device may never see the update; if it caches a read, the CPU may see stale data. The memory management unit (MMU) is typically configured to mark MMIO regions as uncacheable in the page tables, ensuring that every access reaches the device.
