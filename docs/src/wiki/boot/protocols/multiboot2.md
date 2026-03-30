# Multiboot2

Multiboot2 is a boot protocol specification maintained by the GNU project. It defines the contract between a bootloader and an operating system kernel: how the kernel advertises its requirements, what machine state the bootloader establishes before transferring control, and how boot-time information is communicated.

[!side]
See [Bootloaders](bootloaders.md) for how Multiboot2 compares to the Limine protocol, and [JANUS Boot Subsystem](../../project/subsystems/boot/README.md) for how JANUS parses both protocols into a uniform `boot_context_t`.
[/!side]

The protocol is widely supported on x86 machines. GRUB is the canonical implementation, but any Multiboot2-compliant bootloader can load a conforming kernel.

## The Header

The kernel must embed a Multiboot2 header within the first 32 KiB of its binary. The bootloader scans for the magic number `0xE85250D6` at 8-byte-aligned offsets. The header layout is:

| Offset | Field | Value |
|--------|-------|-------|
| 0x00 | Magic | `0xE85250D6` |
| 0x04 | Architecture | `0` for i386 (32-bit protected mode) |
| 0x08 | Header length | Total size of the header in bytes |
| 0x0C | Checksum | `-(magic + architecture + length)`, such that the four fields sum to zero |
| 0x10… | Tags | Request tags, terminated by an end tag |

If the magic number is absent or the checksum does not validate, the bootloader refuses to load the kernel.

## Tags

After the fixed fields, the header contains a sequence of tags. Each tag begins with a type (16 bits), flags (16 bits), and size (32 bits). Tags are 8-byte aligned. The kernel uses tags to request information or declare requirements:

- **Information request (type 1):** Asks the bootloader for specific tag types in the boot information structure — memory map, framebuffer parameters, ELF section headers, etc. The tag body is a list of requested tag type numbers.
- **Framebuffer (type 5):** Requests a specific video mode (width, height, depth).
- **Module alignment (type 6):** Requests that loaded modules be page-aligned.
- **End tag (type 0):** Terminates the header. Required.

## Machine State at Entry

When the bootloader transfers control to the kernel's entry point, the machine is in a precisely defined state:

- **CPU mode:** 32-bit protected mode, paging disabled, A20 gate enabled.
- **Registers:** `EAX` contains the bootloader magic `0x36D76289`. `EBX` contains the physical address of the boot information structure.
- **Segments:** A flat memory model — code and data segments span the full 4 GiB address space.
- **Stack:** Undefined. The kernel must set up its own stack before calling any C code.
- **Interrupts:** Disabled.

Because the processor is in 32-bit mode, a kernel targeting x86_64 must transition to 64-bit long mode itself. This involves setting up initial page tables, enabling PAE, setting the long mode bit in the EFER MSR, enabling paging, and performing a far jump through a 64-bit GDT.

## Boot Information Structure

After loading the kernel, the bootloader places a boot information structure at the address in `EBX`. This structure is a tag list — the same concept as the header tags, but carrying data rather than requests. Common tags include:

| Tag type | Content |
|----------|---------|
| 4 | Basic memory information (lower/upper bounds) |
| 6 | Memory map (array of regions with type and address) |
| 8 | Framebuffer address, dimensions, pitch, pixel format |
| 9 | ELF section headers |
| 14 | ACPI RSDP (old) |
| 15 | ACPI RSDP (new) |

The kernel walks the tag list starting from `EBX`, advancing by each tag's size (rounded up to 8-byte alignment) until it encounters the end tag.
