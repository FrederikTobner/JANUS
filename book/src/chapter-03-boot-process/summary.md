# Summary

You now have a bootable kernel. Let's summarize what we built.

We are creating a bootable ISO image with GRUB as the bootloader.
When launched in QEMU, GRUB loads our kernel into memory and transfers control to it.
The entry point `_start` in assembly switches the CPU to 64-bit long mode and jumps to `kernel_main` in C.
In `kernel_main`, we verify the Multiboot2 magic number to ensure we were booted by a multiboot2-compliant bootloader like GRUB.
If it is valid, we proceed; otherwise, we halt the system gracefully.

## Still Missing

Of course, we can't *see* anything without using the debugger.
For that, we would still need to implement any visible output, such as serial port or VGA text output.
But we are now standing on a solid foundation for the next chapters.

## Challenges

Before moving on, try these exercises to deepen your understanding. Some are research-oriented, others are hands-on implementation.

1. **RISC-V Boot Process** (Research): Read about how RISC-V systems boot. What's the equivalent of the Multiboot2 protocol? How does the CPU start up (machine mode vs supervisor mode vs user mode)? What would need to change in our boot code?

2. **ARM64 Long Mode** (Research): ARM64 has "Exception Levels" (EL0–EL3) instead of x86's ring levels. Research how ARM64 handles the equivalent of our long mode transition. What's the ARM version of page table setup? Write a short comparison (no code needed).

3. **Page Table Portability** (Design): Our page table setup is x86_64-specific (4-level paging with P4/P3/P2/P1). Design a generic abstraction for page tables using a `struct` that could work for both x86_64 and ARM64. What operations would it need to expose? What would be architecture-specific vs portable?

4. **UEFI Boot Transition** (Research): Read the UEFI specification's boot services section. How does UEFI give control to an OS? What information does it provide (compare to Multiboot2's tags)? Why can it skip the 32-bit → 64-bit transition we implemented?

5. **Limine Protocol** (Implementation): Limine is a modern bootloader with a simpler protocol than Multiboot2. Read the Limine protocol specification, then:

- Compare it to Multiboot2. What's simpler? What's missing?
- (Bonus) Implement a Limine boot header for TinyOS.
- (Bonus) Add boot protocol abstraction to support both Multiboot2 and Limine.

1. **Custom Bootloader** (Implementation): Write a tiny bootloader in assembly that:

- Loads your kernel from disk (use BIOS int `0x13`).
- Switches to 32-bit protected mode.
- Passes a custom boot info structure to your kernel.

1. **Bootloader Compatibility** (Research): Test if your kernel works with bootloaders other than GRUB:

- Try GRUB2 vs GRUB Legacy.
- Research: could Syslinux boot a Multiboot2 kernel?
- What would need to change for U-Boot (ARM bootloader)?

---

**Next: [Chapter 4: Input/Output](../chapter-04-io/README.md)**
