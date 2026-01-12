# The Multiboot2 Header

Before we dive deeper, let’s clear up some boot jargon. 
When your PC is powered on, multiple software layers will be traversed to get your kernel running.
The first layer is called the firmware. 
It is the code burned into your motherboard. 
It’s the first thing that runs when you hit the power button. 
There are two main types of firmware on modern Computers. 
The older BIOS (Basic Input Output System) and the more modern UEFI (Unified Extensible Firmware Interface).

The second layer is the boot loader. The boot loader is a small program loaded by the firmware. Its job is to find your kernel and hand over control.

The last layer is your kernel itself. This is the operating system code we will be writing in this book.

They way how we transfer control from the bootloader to our kernel is defined by the boot sequence standard.
It defines how the kernel should be structured so that the bootloader can find and load it correctly and what information the bootloader will provide to the kernel when it is loaded. Additionally it defines the machine state the kernel can expect when it is loaded.

[!side]
**Aside: Why "Multiboot"?**
Back in the 90s, every OS had its own weird boot protocol. Want to boot Linux? Use LILO. Want FreeBSD? Different loader. GRUB said "enough of this nonsense" and created Multiboot—a universal protocol. Now any OS that implements it can boot from GRUB.
[/!side]

We will be using the Multiboot2 standard in this book.

When you power on a PC using a BIOS firmware in combination with the GRUB bootloader, the BIOS will first of all load GRUB from disk. 
GRUB then scans the first 32KB of our kernel binary looking for a magic number—a secret handshake that says "hey, I'm a bootable kernel, load me!"

If you do not provide this magic number GRUB will not recognize your kernel is bootable and will refuse to load it.

```
┌──────────┐    ┌──────────┐    ┌──────────────┐    ┌──────────┐
│   BIOS   │───▶│   GRUB   │───▶│ Scan for     │───▶│  Boot    │
│ Power On │    │  Loads   │    │ 0xe85250d6   │    │  image   │
└──────────┘    └──────────┘    └──────────────┘    └──────────┘
                                       ▲
                                       │
                                 Our Multiboot
                                    Header
```

> **Design Note: Boot Protocol Choices**
>
> There are several ways to boot an operating system:
>
> * **Multiboot2** - Industry standard, well-documented, works everywhere
> * **Limine Protocol** - Modern hobby OS-friendly bootloader with cleaner protocol
> * **Custom bootloader** - Maximum control, maximum work
>
> We're using Multiboot2 for several reasons. It is the most widely supported and understood boot protocol, especially for beginners. 
> Additionally it will provide you with valuable educational insights into the boot process, works on real hardware, and has simple tooling via GRUB.
>
>
> For now, we will focus on understanding the fundamentals.

The Multiboot2 protocol defines the header format our kernel must provide, as well as the information GRUB will pass us when it loads our kernel.
Additionally it defines the machine state we can expect when GRUB transfers control to us.

We will cover the exact contents of the information later when we actually start using it.

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

First, create the directory structure and an empty header file:

```bash
mkdir -p boot/include/boot
touch boot/include/boot/multiboot.h
```

Now let's build the header incrementally.

```c-diff
file: kernel/boot/include/boot/multiboot.h
replace: entire file
---
+ #include "tinyos/types.h"
+
+// Multiboot2 magic value passed by bootloader in EAX
+#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289
+ 
+#endif // BOOT_MULTIBOOT_H
```

Two magic numbers, two different jobs. The header magic (`0xe85250d6`) goes in our assembly—GRUB scans for it to find our kernel. The bootloader magic (`0x36d76289`) is what GRUB passes us in the EAX register as proof it loaded us correctly. Think of them as matching halves of a secret handshake.

Now let's build the Multiboot2 header in assembly. 
This will be 

```x86asm-diff
file: kernel/boot/multiboot.asm
replace: entire file
---
+section .multiboot
+align 8
```

The `.multiboot` section gets its own special spot at the start of our kernel binary. The `align 8` ensures everything starts on an 8-byte boundary—Multiboot2 is picky about alignment, and misaligned headers mean GRUB ignores you completely.


```x86asm-diff
file: kernel/boot/multiboot.asm
after: align 8
---
 align 8
+
+multiboot_start:
+    dd 0xe85250d6
+    
+    dd 0
```

First, we need our old friend the Multiboot2 header magic number—the same `0xe85250d6` we defined in the C header. GRUB scans the first 32KB of our kernel looking for this exact number.

The architecture field tells GRUB what CPU mode we expect: `0` means i386 protected mode (32-bit).

```x86asm-diff
file: kernel/boot/multiboot.asm
after: Architecture definition
---
     dd 0
+    
+    dd multiboot_end - multiboot_start
+    
+    dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))
```

The checksum ensures the header is valid. GRUB verifies that `magic + arch + length + checksum = 0`.

[!side]
The checksum is like a parity bit for the entire header. It catches corruption from bad disk reads or linker bugs.
[/!side]


Ask GRUB for memory information we'll need later:

```x86asm-diff
file: boot/multiboot.asm
after: kernel/boot definition
---
     dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start))
+
+; Information request tag
+align 8
+info_request_start:
+    dw 1                    
+    dw 0                    
+    dd info_request_end - info_request_start
+    
+    dd 4                    
+    dd 6                    
+    dd 8                    
+info_request_end:
```


Every Multiboot2 header must end with this tag:

```x86asm-diff
file: kernel/boot/multiboot.asm
after: info_request_end label
---
 info_request_end:
+
+align 8
+    dw 0                    ; Type = end
+    dw 0                    ; Flags
+    dd 8                    ; Size
+multiboot_end:
```

**What this does:**

* Magic `0xe85250d6` identifies us as Multiboot2
* Checksum validates the header structure  
* Information request asks GRUB for memory details
* End tag terminates the header

GRUB validates the checksum by ensuring `magic + architecture + length + checksum = 0`.

## Why This Matters

Without this header, GRUB won't recognize our kernel. The header is a contract: "I'm a valid kernel, here's what I need from you."

---

**Next: [Boot Entry Assembly](./boot-assembly-64bit.md)**
