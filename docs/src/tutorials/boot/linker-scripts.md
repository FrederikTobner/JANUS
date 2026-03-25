# Writing a Kernel Linker Script

This tutorial builds a linker script for an x86_64 kernel from scratch. The linker script controls where each section of the binary ends up in physical memory, how sections are aligned, and which symbols are available to C code. Getting this right is a prerequisite for everything else — if the Multiboot header is in the wrong place, the bootloader will not find it; if sections are misaligned, paging breaks.

## Prerequisites

Basic familiarity with ELF sections (`.text`, `.data`, `.bss`) and the concept of a linker. Understanding why the kernel loads at 1 MiB helps — see [Firmware](../../wiki/boot/firmware.md) and [Multiboot2](../../wiki/boot/multiboot2.md) for context.

## Why a Custom Linker Script?

For normal user-space programs, the default linker script works because the OS loader handles memory layout. A kernel *is* the OS — there is no loader to fix things up. We need explicit control over:

- **Load address** — the kernel must sit in a region that is free of BIOS tables and hardware mappings.
- **Section ordering** — the Multiboot header must appear within the first 32 KiB of the binary.
- **Alignment** — page-aligned sections enable clean memory permissions (e.g. read-only `.rodata`, executable `.text`).

## Physical Memory Map

The first 1 MiB of x86 physical memory is a patchwork of reserved regions:

```
┌───────────────────────────────────────────────────────┐
│ 0x00000000 – 0x000003FF │ Real-Mode IVT              │
│ 0x00000400 – 0x000004FF │ BIOS Data Area             │
│ 0x00000500 – 0x00007BFF │ Usable (too small)         │
│ 0x00007C00 – 0x00007DFF │ Boot sector                │
│ 0x00007E00 – 0x0009FFFF │ Usable (fragmented)        │
│ 0x000A0000 – 0x000FFFFF │ Video memory / BIOS ROMs   │
├───────────────────────────────────────────────────────┤
│ 0x00100000 – ...        │ Kernel loads here ←        │
└───────────────────────────────────────────────────────┘
```

Loading the kernel at `0x100000` (1 MiB) avoids all of this. It is a conventional choice on x86 dating back to early Linux.

## Building the Script Incrementally

Create `linker.ld` and start with the entry point and load address:

```ld
ENTRY(_start)

SECTIONS {
    . = 0x100000;
}
```

`ENTRY(_start)` tells the linker which symbol is the first instruction to execute. `. = 0x100000` sets the location counter to 1 MiB — everything that follows is placed from this address onwards.

### Multiboot Header

GRUB scans the first 32 KiB for the Multiboot2 magic. The header section must appear first, 8-byte aligned:

```ld
SECTIONS {
    . = 0x100000;

    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
}
```

`*(.multiboot)` pulls in the `.multiboot` input section from every object file. The wildcard `*` is necessary because the linker does not know which object file contains it. `ALIGN(8)` is a Multiboot2 requirement.

### Code

Executable code goes into `.text`, page-aligned so we can later mark it executable but non-writable:

```ld
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }
```

The `.text.*` pattern catches compiler-generated subsections like `.text.unlikely` (cold code) or `.text.startup`.

### Read-Only Data

String literals, `const` globals, and jump tables live in `.rodata`:

```ld
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
```

Page-aligning this section separately from `.text` lets us map it as readable but not executable — a useful security property even in kernel space.

### Initialised Data

Global and static variables with compile-time initialisers go into `.data`:

```ld
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
```

These values are stored in the ELF binary and loaded into memory by the bootloader.

### Uninitialised Data (BSS)

BSS holds variables that are zero-initialised. They take no space in the binary itself — the bootloader zeroes the region at load time:

```ld
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
```

`COMMON` collects tentative C definitions (globals declared without an initialiser in multiple translation units). Including it here ensures they end up in the zeroed region rather than floating unplaced.

### Kernel Boundary Symbols

Expose the kernel's memory footprint to C so that a future memory allocator knows where free memory begins:

```ld
    kernel_start = 0x100000;
    kernel_end   = .;
    kernel_size  = kernel_end - kernel_start;
```

Use them in C via `extern`:

```c
extern char kernel_end;
void *first_free_byte = &kernel_end;
```

Note: `kernel_end` is a *symbol*, not a variable. It has an address but no storage. Taking its address (`&kernel_end`) gives the first byte after the kernel image.

## Complete Linker Script

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
    kernel_end   = .;
    kernel_size  = kernel_end - kernel_start;
}
```

## Verification

After building, inspect the resulting ELF with `readelf -l`:

```
Program Headers:
  Type   Offset             VirtAddr           PhysAddr           Flags  Align
  LOAD   0x0000000000001000 0x0000000000100000 0x0000000000100000 R      0x1000
  LOAD   0x0000000000002000 0x0000000000101000 0x0000000000101000 R E    0x1000
  LOAD   0x0000000000003000 0x0000000000102000 0x0000000000102000 R      0x1000
  LOAD   0x0000000000001000 0x0000000000103000 0x0000000000103000 RW     0x1000

Section to Segment mapping:
  00     .multiboot
  01     .text
  02     .rodata
  03     .bss
```

Things to verify:

- `.multiboot` starts at `0x100000` and is read-only (`R`).
- `.text` has execute permission (`R E`).
- `.rodata` is read-only, no execute.
- `.bss` is read-write (`RW`), no execute.
- Every LOAD segment is aligned to `0x1000` (4 KiB).

If any section has unexpected permissions or appears at the wrong address, the linker script has a mistake.

## Key Takeaways

- **The location counter (`.`) is the central concept.** Every section placement is relative to it.
- **Alignment matters for paging.** Page-aligned sections enable per-section memory permissions.
- **Wildcards are unavoidable.** The compiler may split code and data across subsections like `.text.startup` or `.rodata.str1.1`. Catching them with `*(.text.*)` prevents silent data loss.
- **Linker symbols are addresses, not variables.** Access them with `&`, never by value.
