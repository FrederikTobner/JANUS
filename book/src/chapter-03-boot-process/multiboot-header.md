# The Multiboot2 Header

When you power on a PC, the BIOS loads GRUB from disk. GRUB then scans the first 32KB of our kernel binary looking for a magic number—a secret handshake that says "hey, I'm a bootable kernel, load me!"

[!side]
The magic number 0xe85250d6 was chosen randomly. It's unlikely to appear by accident in executable code.
[/!side]

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

[!side]
**Aside: Why "Multiboot"?**
Back in the 90s, every OS had its own weird boot protocol. Want to boot Linux? Use LILO. Want FreeBSD? Different loader. GRUB said "enough of this nonsense" and created Multiboot—a universal protocol. Now any OS that implements it can boot from GRUB.
[/!side]

> **Design Note: Boot Protocol Choices**
>
> There are several ways to boot an operating system:
>
> - **BIOS + Multiboot2** (what we're using) - Industry standard, well-documented, works everywhere
> - **UEFI** - Modern standard, boots directly into 64-bit mode, more complex setup
> - **Limine Protocol** - Modern hobby OS-friendly bootloader with cleaner protocol
> - **Custom bootloader** - Maximum control, maximum work
>
> We're using Multiboot2 for this book because:
>
> 1. **It's the standard** - Works with GRUB, widely understood, extensive documentation
> 2. **Educational value** - The 32-bit to 64-bit transition we'll implement teaches important concepts about CPU modes and paging. UEFI skips this by booting directly into 64-bit mode, which is convenient but hides crucial details.
> 3. **Real hardware support** - Works on actual PCs, not just emulators
> 4. **Simple tooling** - GRUB handles all the complexity of disk formats, filesystems, etc.
>
> **What about UEFI?** It's the modern replacement for BIOS and has real advantages (better hardware discovery, graphics modes, Secure Boot support). But it's significantly more complex for beginners. Once you understand booting with Multiboot2, adding UEFI support is a natural next step—you'll know exactly what the bootloader needs to provide and why.
>
> For now, focus on understanding the fundamentals. Boot protocol abstraction can come later once you have a working kernel.

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

First, create the directory structure and an empty header file:

```bash
mkdir -p boot/include/boot
touch boot/include/boot/multiboot.h
```

Now let's build the header incrementally.

#### Step 1: Header Guards and Basic Structure

```c-diff
file: boot/include/boot/multiboot.h
replace: entire file
---
+#ifndef BOOT_MULTIBOOT_H
+#define BOOT_MULTIBOOT_H
+
+#include <stdint.h>
+
+#endif // BOOT_MULTIBOOT_H
```

#### Step 2: Add Multiboot2 Magic Numbers

```c-diff
file: boot/include/boot/multiboot.h
after: #include <stdint.h>
---
 #include <stdint.h>
+
+// Multiboot2 magic value passed by bootloader in EAX
+#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289
+
+// Multiboot2 header magic (in the header itself)
+#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
 
 #endif // BOOT_MULTIBOOT_H
```

Two magic numbers, two different jobs. The header magic (`0xe85250d6`) goes in our assembly—GRUB scans for it to find our kernel. The bootloader magic (`0x36d76289`) is what GRUB passes us in the EAX register as proof it loaded us correctly. Think of them as matching halves of a secret handshake.

#### Step 3: Add Architecture Constants

```c-diff
file: boot/include/boot/multiboot.h
after: MULTIBOOT2_HEADER_MAGIC definition
---
 #define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
+
+// Architecture types
+#define MULTIBOOT2_ARCHITECTURE_I386 0
 
 #endif // BOOT_MULTIBOOT_H
```

Tells GRUB we want to start in 32-bit protected mode. Yes, we're building a 64-bit kernel, but we'll handle the upgrade ourselves in assembly. GRUB gives us a solid 32-bit foundation and we take it from there.

#### Step 4: Forward Declaration for Future Use

```c-diff
file: boot/include/boot/multiboot.h
after: MULTIBOOT2_ARCHITECTURE_I386 definition
---
 #define MULTIBOOT2_ARCHITECTURE_I386 0
+
+// Forward declaration - full definition later
+struct multiboot_info;
 
 #endif // BOOT_MULTIBOOT_H
```

A promise to the compiler: "Trust me, this struct exists, I'll show you the details later." Forward declarations let us use `struct multiboot_info*` in function signatures without defining the entire structure yet. We'll fill in the real definition once we need to actually parse the multiboot data.

### Create the Assembly Header

Now let's build the Multiboot2 header in assembly. Create an empty file:

```bash
touch boot/multiboot.asm
```

#### Step 1: Section and Alignment

```asm-diff
file: boot/multiboot.asm
replace: entire file
---
+; Multiboot2 header - must be in first 32KB of kernel image
+section .multiboot
+align 8
```

The `.multiboot` section gets its own VIP spot at the start of our kernel binary (thanks to our linker script later). The `align 8` ensures everything starts on an 8-byte boundary—Multiboot2 is picky about alignment, and misaligned headers mean GRUB ignores you completely.

#### Step 2: Magic Number and Architecture

```asm-diff
file: boot/multiboot.asm
after: align 8
---
 align 8
+
+multiboot_start:
+    ; Magic number - identifies this as Multiboot2
+    dd 0xe85250d6
+    
+    ; Architecture: 0 = i386 protected mode
+    dd 0
```

First, we need our old friend the Multiboot2 header magic number—the same `0xe85250d6` we defined in the C header. GRUB scans the first 32KB of our kernel looking for this exact number.

The architecture field tells GRUB what CPU mode we expect: `0` means i386 protected mode (32-bit). Even though we'll transition to 64-bit later, GRUB starts us in 32-bit mode and we handle the upgrade ourselves.

#### Step 3: Header Length and Checksum

```asm-diff
file: boot/multiboot.asm
after: Architecture definition
---
     dd 0
+    
+    ; Header length
+    dd multiboot_end - multiboot_start
+    
+    ; Checksum: -(magic + arch + length)
+    dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))
```

The checksum ensures the header is valid. GRUB verifies that `magic + arch + length + checksum = 0`.

[!side]
The checksum is like a parity bit for the entire header. It catches corruption from bad disk reads or linker bugs.
[/!side]

#### Step 4: Information Request Tag

Ask GRUB for memory information we'll need later:

```asm-diff
file: boot/multiboot.asm
after: Checksum definition
---
     dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))
+
+; Information request tag
+align 8
+info_request_start:
+    dw 1                    ; Type = information request
+    dw 0                    ; Flags = required
+    dd info_request_end - info_request_start
+    
+    dd 4                    ; Request: basic memory info
+    dd 6                    ; Request: memory map
+    dd 8                    ; Request: framebuffer info (graphics display info)
+info_request_end:
```

#### Step 5: End Tag (Required Terminator)

Every Multiboot2 header must end with this tag:

```asm-diff
file: boot/multiboot.asm
after: info_request_end label
---
 info_request_end:
+
+; End tag (required)
+align 8
+    dw 0                    ; Type = end
+    dw 0                    ; Flags
+    dd 8                    ; Size
+multiboot_end:
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
