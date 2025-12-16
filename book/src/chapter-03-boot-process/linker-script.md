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

## Memory Layout Strategy

Our kernel loads at **1MB** (physical address `0x100000`). Why?

```
Physical Memory Map:
┌─────────────────────────────────────┐
│ 0x00000000 - 0x000003FF │ Real Mode IVT         │ BIOS territory
│ 0x00000400 - 0x000004FF │ BIOS Data Area        │ Don't touch!
│ 0x00000500 - 0x00007BFF │ Usable                │ Too small for kernel
│ 0x00007C00 - 0x00007DFF │ Bootloader            │ GRUB lives here
│ 0x00007E00 - 0x0009FFFF │ Usable                │ Fragmented, messy
│ 0x000A0000 - 0x000FFFFF │ Video/BIOS            │ Hardware mapped
├─────────────────────────────────────┤
│ 0x00100000 - ...        │ KERNEL LOADS HERE     │ ← Clean, contiguous
└─────────────────────────────────────┘
```

[!side]
IVT = Interrupt Vector Table from Real Mode (16-bit). The BIOS uses it, we don't, but we can't overwrite it.
[/!side]

The first 1MB is a minefield of BIOS tables, video memory, and historical baggage. Loading at 1MB gives us a clean slate.

[!side]
1MB was "high memory" in 1981. Now it's the standard kernel load address for x86.
[/!side]

## Section Organization

Our kernel has several sections, and their order matters:

1. **`.multiboot`** — GRUB scans the first 32KB for this. Must be first!
2. **`.text`** — Executable code (read-only, executable)
3. **`.rodata`** — Read-only data (string literals, const variables)
4. **`.data`** — Initialized global/static variables
5. **`.bss`** — Uninitialized data (zeroed at boot, saves space in binary)

[!side]
BSS = Block Started by Symbol (historical IBM assembler term). Saves disk space—no need to store zeros.
[/!side]

## Building the Linker Script

Let's build our linker script incrementally. Create `kernel/linker.ld` and we'll add to it step by step.

### Step 1: Entry Point and Load Address

Start with the skeleton—where execution begins and where the kernel loads:

```ld-diff
file: kernel/linker.ld
---
+ENTRY(_start)
+
+SECTIONS {
+    . = 0x100000;
+}
```

**What this does:**

- `ENTRY(_start)` — Execution starts at our `_start` label from boot.asm
- `SECTIONS { }` — Container for all our memory layout
- `. = 0x100000` — Load address (the **location counter** `.` tracks our position)

The location counter is like a cursor. As we add sections, it moves forward automatically.

### Step 2: Multiboot Header Section

GRUB scans the first 32KB for the Multiboot header. Add it right after the load address:

```ld-diff
file: kernel/linker.ld
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

### Step 3: Code Section

Add the executable code section after `.multiboot`:

```ld-diff
file: kernel/linker.ld
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

**What this does:**

- `.text` — Executable instructions
- `ALIGN(4K)` — Page-aligned (4096 bytes, x86-64 page size)
- `*(.text.*)` — Catches compiler subsections (`.text.startup`, `.text.hot`, etc.)

Page alignment lets us set memory permissions cleanly (executable vs. non-executable).

### Step 4: Read-Only Data

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

**What this does:**

- `.rodata` — Read-only data (strings, const globals)
- Separate from `.text` so we can mark it non-executable (prevents code injection)

### Step 5: Initialized Data

Add the initialized data section after `.rodata`:

```ld-diff
file: kernel/linker.ld
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

**What this does:**

- `.data` — Initialized variables (values stored in ELF, loaded into memory)
- Read-write permissions

### Step 6: Uninitialized Data (BSS)

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

**What this does:**

- `.bss` — Uninitialized data (zeroed by bootloader)
- `*(COMMON)` — Tentative definitions (old C feature for globals declared in multiple files)
- Not stored in ELF—saves disk space

### Step 7: Kernel Boundary Symbols

Finally, add these symbols at the end of the `SECTIONS` block (after `.bss`):

```ld-diff
file: kernel/linker.ld
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

**What this does:**

- `kernel_start` — Address where kernel begins
- `kernel_end = .;` — Current location (end of all sections)
- `kernel_size` — Total size

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
