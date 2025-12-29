# The Linker Script

We have assembly files. We'll have C files. But how do we tell the linker how to combine them into a bootable kernel? That's where the linker script comes in.

Linker scripts are weird. They're not C. They're not assembly. They're their own strange declarative language that tells `ld` (the linker): "put this section here, that section there, and make sure everything lines up." Think of it as interior design for memory—you're telling the linker where to place the furniture, except the furniture is code and if you put it in the wrong place, the CPU throws a tantrum and reboots.

[!side]
Linker script syntax is arcane. Don't try to memorize it. Copy, modify, and reference as needed.
[/!side]

> **The Crux: Why Can't the Linker Just Figure It Out?**
>
> For normal programs, it can! The default linker script works fine because the OS loader handles the details. But we *are* the OS. We need exact control over:
>
> - Where in memory our code lives
> - The order of sections (Multiboot header must be first!)
> - Alignment (page boundaries matter for memory management)
> - Section permissions (read-only vs. read-write)

Our **memory layout** will look like this:


```
Physical Memory Map:
┌─────────────────────────────────────────────────┐
│ 0x00000000 - 0x000003FF │ Real Mode IVT         │ BIOS territory
│ 0x00000400 - 0x000004FF │ BIOS Data Area        │ Don't touch!
│ 0x00000500 - 0x00007BFF │ Usable                │ Too small for kernel
│ 0x00007C00 - 0x00007DFF │ Bootloader            │ GRUB lives here
│ 0x00007E00 - 0x0009FFFF │ Usable                │ Fragmented, messy
│ 0x000A0000 - 0x000FFFFF │ Video/BIOS            │ Hardware mapped
├─────────────────────────────────────────────────┤
│ 0x00100000 - ...        │ KERNEL LOADS HERE     │ ← Clean, contiguous
└─────────────────────────────────────────────────┘
```

> TODO: Make sure we do this in chapter 4 otherwise remove

[!side]
IVT = Interrupt Vector Table from Real Mode (16-bit). The BIOS uses this, to configure hardware interrupts.
We will touch on this later in chapter 4 when we will implement the handling of input and output.
[/!side]

As you may have spotted from the diagram, we load our kernel at **1MB**. This is a traditional location for kernels on x86 architecture.

The reason for this is that the first 1MB is a minefield of BIOS tables, video memory, and historical baggage. 
Loading at 1MB and ignoring the other regions that would in theory also be usable makes our life much simpler.

Our linker script will define these sections in order:

1. **`.multiboot`** — GRUB scans the first 32KB for this section. We need to insure it is placed in the very beginning of our binary. 
2. **`.text`** — Executable code (read-only, executable)
3. **`.rodata`** — Read-only data (string literals, const variables)
4. **`.data`** — Initialized global/static variables
5. **`.bss`** — Uninitialized data (zeroed at boot, saves space in binary)

[!side]
BSS = Block Started by Symbol (historical IBM assembler term). Saves disk space—no need to store zeros.
[/!side]

## Building the Linker Script

Let's build our linker script incrementally. Create `kernel/core/linker.ld` and we'll add to it step by step.


Start with the skeleton—where execution begins and where the kernel loads:

```ld-diff
file: kernel/core/linker.ld
after: entire file
---
+ENTRY(_start)
+
+SECTIONS {
+    . = 0x100000;
+}
```
Fist of all we state that the execution starts at our `_start` label from boot.asm 
by using the `ENTRY` directive. Next we define the `SECTIONS` block, which contains all our memory layout.
Then we set the location counter `.` to `0x100000` (1MB)—our load address.


GRUB scans the first 32KB for the Multiboot header. Add it right after the load address:

```ld-diff
file: kernel/core/linker.ld
after: . = 0x100000;
---
     . = 0x100000;
+    
+    .multiboot ALIGN(8) : {
+        *(.multiboot)
+    }
 }
```

**What this does:**

- `.multiboot` — Section name
- `ALIGN(8)` — 8-byte alignment (Multiboot2 requirement)
- `*(.multiboot)` — Pull in `.multiboot` sections from all object files (`*` = wildcard)

This must come first or GRUB won't find it.

Add the executable code section after `.multiboot`:

```ld-diff
file: kernel/core/linker.ld
after: .multiboot section
---
     .multiboot ALIGN(8) : {
         *(.multiboot)
     }
+    
+    .text ALIGN(4K) : {
+        *(.text)
+        *(.text.*)
+    }
 }
```

In order to allign to page boundaries, we use `ALIGN(4K)` (4096 bytes).

Page alignment lets us set memory permissions cleanly (executable vs. non-executable).

Add the read-only data section after `.text`:

```ld-diff
file: kernel/linker.ld
after: .text section
---
     .text ALIGN(4K) : {
         *(.text)
         *(.text.*)
     }
+    
+    .rodata ALIGN(4K) : {
+        *(.rodata)
+        *(.rodata.*)
+    }
 }
```

The `.rodata` section holds read-only data like string literals and `const` globals.

Next we will add the data section for initialized variables.
```ld-diff
file: kernel/core/linker.ld
after: .rodata section
---
     .rodata ALIGN(4K) : {
         *(.rodata)
         *(.rodata.*)
     }
+    
+    .data ALIGN(4K) : {
+        *(.data)
+        *(.data.*)
+    }
 }
```

The vakues stored in `.data` are loaded from the ELF file into memory at boot.

Add the BSS section after `.data`:

```ld-diff
file: kernel/linker.ld
after: .data section
---
     .data ALIGN(4K) : {
         *(.data)
         *(.data.*)
     }
+    
+    .bss ALIGN(4K) : {
+        *(COMMON)
+        *(.bss)
+        *(.bss.*)
+    }
 }
```

The `.bss` holds uninitialized data, that the bootloader zeroes out for us.

Finally, add these symbols at the end of the `SECTIONS` block (after `.bss`):

```ld-diff
file: kernel/core/linker.ld
after: .bss section
---
     .bss ALIGN(4K) : {
         *(COMMON)
         *(.bss)
         *(.bss.*)
     }
+    
+    kernel_start = 0x100000;
+    kernel_end = .;
+    kernel_size = kernel_end - kernel_start;
 }
```

First we set the `kernel_start` symbol to `0x100000` (our load address). 
Then we set `kernel_end` to the current location counter `.` (the end of all sections). 
Finally, we compute `kernel_size` as the difference between `kernel_end` and `kernel_start`.

These become usable in C:

```c
extern char kernel_end;
void *end = &kernel_end;  // Get the address
```

That's the complete linker script! Save it as `kernel/linker.ld`.

## Complete Reference

Here's the final linker script for reference:

```ld
ENTRY(_start)

SECTIONS {
    . = 0x100000;
    
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    kernel_start = 0x100000;
    kernel_end = .;
    kernel_size = kernel_end - kernel_start;
}
```

## Checkpoint: Understanding the Linker Script

Before moving on, make sure you understand:

**The Big Picture:**

- Why we need exact memory control (we are the OS, no loader to help us)
- Why 1MB is our starting address (BIOS owns the first 1MB)
- Why section order matters (Multiboot header must be first for GRUB to find it)

**Key Concepts:**

- **Location counter (`.`)**: Tracks where we're placing things in memory
- **ALIGN(4K)**: Aligns sections to page boundaries (4096 bytes)
- **Wildcards (`*`)**: Pulls in sections from all object files
- **Section permissions**: .text is executable, .rodata is read-only, .data/.bss are writable

**Test Your Understanding:**

- What would happen if `.multiboot` wasn't first? (GRUB wouldn't find it, boot fails)
- Why align to 4KB? (Page size on x86-64, needed for memory protection)
- Why is `.bss` separate from `.data`? (Saves disk space—uninitialized = no zeros stored)

[!side]
If any of these feel unclear, re-read the sections above. The linker script controls your entire memory layout—worth understanding deeply.
[/!side]

## Verification

After building, verify the layout:

```bash
readelf -l build/kernel.elf
```

Output:

```
Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000001000 0x0000000000100000 0x0000000000100000
                 0x0000000000000030 0x0000000000000030  R      0x1000
  LOAD           0x0000000000002000 0x0000000000101000 0x0000000000101000
                 0x00000000000000da 0x00000000000000da  R E    0x1000
  LOAD           0x0000000000003000 0x0000000000102000 0x0000000000102000
                 0x000000000000001a 0x000000000000001a  R      0x1000
  LOAD           0x0000000000000000 0x0000000000103000 0x0000000000103000
                 0x0000000000000000 0x0000000000007000  RW     0x1000
```

All sections start at 0x100000 (1MB) with 4KB alignment.

---

**Next: [The CMake Build System →](build-system.md)**
