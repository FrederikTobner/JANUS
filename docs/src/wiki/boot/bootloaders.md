# Bootloaders

A bootloader is the intermediate software that bridges firmware and the operating system kernel.
The firmware knows how to initialise hardware and load a small program from disk; the bootloader uses those firmware services to locate the kernel binary, load it into memory, configure the CPU and memory environment, according the the boot protocol, and then transfers control to the kernel's entry point.

## Why Not Boot Directly?

The firmware's native loading mechanism is rudimentary. BIOS loads exactly 512 bytes from the first disk sector. UEFI loads a PE executable from a FAT32 partition. Neither understands ELF binaries, kernel command lines, initial ramdisks, or framebuffer configuration. The bootloader fills this gap — it understands the kernel's binary format, sets up the environment the kernel expects, and passes structured information, like memory maps, framebuffer parameters, and boot command line arguments, to the kernel in a well-defined format.

## Boot Protocols

A boot protocol is the contract between the bootloader and the kernel. It defines the state of the machine at the moment control transfers to the kernel's entry point: which CPU mode is active, how the kernel is mapped in memory, and how boot-time information is communicated.

### Multiboot2

The Multiboot2 specification, maintained by the GNU project, is a widely supported protocol for x86 machines. The kernel embeds a Multiboot2 header — a structured sequence of tags that declares what the kernel requires (memory map, framebuffer, module loading). A Multiboot2-compliant bootloader (typically GRUB) reads these tags, fulfils the requests, and enters the kernel's entry point in 32-bit protected mode with the machine in a precisely defined state.

Boot information is passed to the kernel as a tag list at a pointer supplied in register `ebx`. The kernel walks this tag list to discover the memory map, framebuffer parameters, and any loaded modules.

Multiboot2 is BIOS-oriented — it enters the kernel in 32-bit protected mode, and the kernel must set up 64-bit long mode itself. On UEFI machines, GRUB handles the firmware interaction internally and still enters the kernel in the Multiboot2-defined state.

### Limine

The Limine boot protocol is a newer, simpler alternative that supports both x86_64 and aarch64. Rather than a tag list, Limine uses a request/response model: the kernel defines request structures as global symbols, and the bootloader populates them before transferring control. The protocol enters the kernel in 64-bit long mode (x86_64) or EL1 (aarch64) with paging already enabled and a higher-half direct map (HHDM) providing access to all physical memory.

Limine is firmware-agnostic — the same protocol works whether the underlying firmware is BIOS or UEFI. It handles the mode switch and page table setup internally, so the kernel starts in a significantly more comfortable environment than Multiboot2 provides.

[!side]
See [JANUS Boot Subsystem](../../project/subsystems/boot/README.md) for how JANUS handles both protocols through a uniform `boot_init()` interface.
[/!side]
