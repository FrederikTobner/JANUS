# The Multiboot2 Header

When you power on a PC, the BIOS loads GRUB from disk. GRUB then scans the first 32KB of our kernel binary looking for a magic number—a secret handshake that says "hey, I'm a bootable kernel, load me!"

No magic number? GRUB ignores you. Your kernel sits there on disk, lifeless.

```
Boot Sequence:
┌──────────┐    ┌──────────┐    ┌──────────────┐    ┌──────────┐
│   BIOS   │───▶│   GRUB   │───▶│ Scan for     │───▶│  Found!  │
│ Power On │    │  Loads   │    │ 0xe85250d6   │    │ Load it  │
└──────────┘    └──────────┘    └──────────────┘    └──────────┘
                                       ▲
                                       │
                                 Our Multiboot
                                    Header
```

> **Aside: Why "Multiboot"?**
>
> Back in the 90s, every OS had its own weird boot protocol. Want to boot Linux? Use LILO. Want FreeBSD? Different loader. GRUB said "enough of this nonsense" and created Multiboot—a universal protocol. Now any OS that implements it can boot from GRUB.

## The Multiboot2 Contract

Multiboot2 defines:

1. **Header format**: Magic number, architecture, checksum
2. **Boot state**: CPU mode, registers, memory state when we're loaded
3. **Information tags**: What data the bootloader provides (memory map, etc.)

### Multiboot2 Header Structure

```
Multiboot2 Header Layout (must be in first 32KB):
┌─────────────────────────────────────────┐
│  Offset  │  Field         │  Value      │
├─────────────────────────────────────────┤
│  0x00    │  Magic         │  0xe85250d6 │ ← GRUB looks for this
│  0x04    │  Architecture  │  0 (i386)   │
│  0x08    │  Header Length │  (computed) │
│  0x0C    │  Checksum      │  (computed) │
├─────────────────────────────────────────┤
│          │  Tags...       │             │ ← Optional info requests
├─────────────────────────────────────────┤
│          │  End Tag (0)   │             │ ← Required terminator
└─────────────────────────────────────────┘
```

> **TODO: Hand-drawn illustration idea**
> Draw GRUB as a detective with a magnifying glass scanning through a pile of binary (represented as 1s and 0s), suddenly spotting the magic number 0xe85250d6 and exclaiming "Aha! Found it!" with the kernel hiding behind the pile looking nervous.

### Create the Multiboot Header File

First, create `boot/include/boot/multiboot.h` to define the constants we'll use:

```bash
mkdir -p boot/include/boot
```

```c
// boot/include/boot/multiboot.h
#ifndef BOOT_MULTIBOOT_H
#define BOOT_MULTIBOOT_H

#include <stdint.h>

// Multiboot2 magic value passed by bootloader in EAX
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

// Multiboot2 header magic (in the header itself)
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

// Architecture types
#define MULTIBOOT2_ARCHITECTURE_I386 0

// Forward declaration - full definition later
struct multiboot_info;

#endif // BOOT_MULTIBOOT_H
```

For now, we just need the magic numbers. We'll expand this header when we start parsing multiboot information.

### Create the Assembly Header

Now create `boot/multiboot.asm`:

```nasm
; Multiboot2 header - must be in first 32KB of kernel image
section .multiboot
align 8

multiboot_start:
    ; Magic number - identifies this as Multiboot2
    dd 0xe85250d6
    
    ; Architecture: 0 = i386 protected mode
    dd 0
    
    ; Header length
    dd multiboot_end - multiboot_start
    
    ; Checksum: -(magic + arch + length)
    dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))

; Information request tag
align 8
info_request_start:
    dw 1                    ; Type = information request
    dw 0                    ; Flags = required
    dd info_request_end - info_request_start
    
    dd 4                    ; Request: basic memory info
    dd 6                    ; Request: memory map
    dd 8                    ; Request: framebuffer info (graphics display info)
info_request_end:

; End tag (required)
align 8
    dw 0                    ; Type = end
    dw 0                    ; Flags
    dd 8                    ; Size
multiboot_end:
```

**What this does:**

- Magic `0xe85250d6` identifies us as Multiboot2
- Checksum validates the header structure
- Information request asks GRUB for memory details
- End tag terminates the header

GRUB validates the checksum by ensuring `magic + architecture + length + checksum = 0`.

## Why This Matters

Without this header, GRUB won't recognize our kernel. The header is a contract: "I'm a valid kernel, here's what I need from you."

---

**Next: [Boot Entry Assembly →](boot-assembly.md)**
