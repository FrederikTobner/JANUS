# Summary

Let's take stock. We now have:

- **Project structure** — directories organized, modules separated
- **Standard library headers** — `stdint.h`, `stddef.h`, `stdbool.h`
- **Multiboot2 header** — the secret handshake GRUB needs
- **Boot assembly** — stack setup, register preservation, C function call
- **Linker script** — memory layout and section organization
- **CMake build system** — automated compilation with Ninja
- **Kernel entry point** — C function with Multiboot verification

We can now build a bootable kernel with a single command. The kernel boots, verifies it was loaded correctly.

## Current Capabilities

You now have a complete bootable kernel! Let's verify:

```bash
# Check kernel file type
file build/kernel.elf
```

Output:

```
build/kernel.elf: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), 
statically linked, with debug_info, not stripped
```

```bash
# Check entry point
readelf -h build/kernel.elf | grep Entry
```

Output:

```
  Entry point address:               0x101000
```

```bash
# Verify symbols
nm build/kernel.elf | grep -E "(start|kernel_main)"
```

Output:

```
0000000000101000 T _start
0000000000101090 T kernel_main
```

**What this means:**

- Build produces valid `kernel.elf` executable
- Entry point correctly set to `_start` (0x101000) from boot assembly
- Kernel loads at 1MB (0x100000) with proper alignment
- Multiboot2 header in correct location
- `kernel_main` symbol present and callable
- Incremental builds work with dependency tracking
- Kernel can verify Multiboot magic number
- Graceful halt if bootloader is invalid

## Still Missing

Of course, we can't *see* any of this yet. For that, we still need:

- Any visible output (serial or VGA)
- Memory management
- Interrupt handling
- Actually using the Multiboot information

But we're standing on a solid foundation. Everything else builds from here

[!side]
You now understand more about x86_64 booting than most programmers ever will. Nice work!
[/!side]

> **Challenges**
>
> Before moving on, try these exercises to deepen your understanding. Some are > research-oriented, others are hands-on implementation.
>
> 1. **RISC-V Boot Process** (Research): Read about how RISC-V systems boot. > What's the equivalent of the Multiboot2 protocol? How does the CPU start up >( machine mode vs supervisor mode vs user mode)? What would need to change in > our boot code?
>
> 2. **ARM64 Long Mode** (Research): ARM64 has "Exception Levels" (EL0-EL3) > instead of x86's ring levels. Research how ARM64 handles the equivalent of > our long mode transition. What's the ARM version of page table setup? Write a  >short comparison (no code needed).
>
> 3. **Page Table Portability** (Design): Our page table setup is > x86_64-specific (4-level paging with P4/P3/P2/P1). Design a generic > abstraction for pagetables using a struct that could work for both x86_64 a nd >ARM64. What operations would it need to expose? What would be > architecture-specific vs portable?
>
> 4. **UEFI Boot Transition** (Research): Read the UEFI specification's boot > services section. How does UEFI give control to an OS? What information does > it provide (compare to Multiboot2's tags)? Why can it skip the 32-bit→64-bit > transition we had to implement?
>
> 5. **Limine Protocol** (Implementation): Limine is a modern bootloader with a  >simpler protocol than Multiboot2. Read the Limine protocol specification, > then:
>
> - Compare it to Multiboot2. What's simpler? What's missing?
> - (Bonus) Implement a Limine boot header for TinyOS
> - (Bonus) Add boot protocol abstraction to support both Multiboot2 and > Limine
>
> 6. **Custom Bootloader** (Implementation): Write a tiny bootloader in > assembly that:
>
> - Loads your kernel from disk (use BIOS int 0x13)
> - Switches to 32-bit protected mode
> - Passes a custom boot info structure to your kernel
> - (Hint: You'll need to write it to the boot sector and create a disk > image)
>
> 7. **Bootloader Compatibility** (Research): Test if your kernel works with > bootloaders other than GRUB:
>     - Try GRUB2 vs GRUB Legacy
>     - Research: Could syslinux boot a Multiboot2 kernel?
>     - What would need to change for U-Boot (ARM bootloader)?

---

**Next: [Chapter 3: Input/Output](../chapter-03-io/README.md)**
