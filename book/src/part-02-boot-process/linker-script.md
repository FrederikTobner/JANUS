# The Linker Script

We have assembly files. We'll have C files. But how do we tell the linker how to combine them into a bootable kernel? That's where the linker script comes in.

Linker scripts are weird. They're not C. They're not assembly. They're their own strange declarative language that tells `ld` (the linker): "put this section here, that section there, and make sure everything lines up."

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

> **Aside: What's the IVT?**
>
> IVT stands for Interrupt Vector Table—a table used in Real Mode (16-bit mode the CPU starts in at power-on) that contains addresses of interrupt handlers. Each entry is 4 bytes (segment:offset pair). It's at address 0x0 for historical reasons dating back to the original 8086 processor. We don't use it (we're in protected/long mode), but the BIOS does, so we leave it alone.

The first 1MB is a minefield of BIOS tables, video memory, and historical baggage. Loading at 1MB gives us a clean slate.

## Section Organization

Our kernel has several sections, and their order matters:

1. **`.multiboot`** — GRUB scans the first 32KB for this. Must be first!
2. **`.text`** — Executable code (read-only, executable)
3. **`.rodata`** — Read-only data (string literals, const variables)
4. **`.data`** — Initialized global/static variables
5. **`.bss`** — Uninitialized data (zeroed at boot, saves space in binary)

## Creating the Linker Script

Create the file `kernel/linker.ld`:

```ld
/* Entry point symbol defined in boot/boot.asm */
ENTRY(_start)

SECTIONS {
    /* Kernel loads at 1MB - above BIOS reserved area */
    . = 0x100000;
    
    /* Multiboot header must be at the very start */
    .multiboot ALIGN(8) : {
        *(.multiboot)
    }
    
    /* Code section - executable instructions */
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }
    
    /* Read-only data - string literals, const variables */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized data - global/static variables with initial values */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized data - global/static variables zeroed at boot */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* Symbols for kernel bounds - useful for memory management */
    kernel_start = 0x100000;
    kernel_end = .;
    kernel_size = kernel_end - kernel_start;
}
```

## Linker Script Explained

**Location counter (`.`)**: The magic `.` variable tracks our current position in memory. `. = 0x100000` says "start placing things at 1MB."

**Section syntax**: `.text ALIGN(4K) : { *(.text) }` means:

- Create a section named `.text`
- Align it to a 4KB boundary (page size)
- Include all `.text` sections from all input files (`*(.text)`)

**Why `ALIGN(4K)`?** Pages are 4KB on x86-64. Aligning sections to page boundaries lets us set different permissions later (code is executable, data is not).

**Wildcard patterns**: `*(.text.*)` catches compiler-generated sections like `.text.startup` or `.text.hot` (optimization artifacts).

**COMMON**: Old C quirk. Multiple files can define the same uninitialized global, and the linker merges them. We put these in `.bss`.

**Symbols**: `kernel_end = .;` creates a symbol we can reference in C:

```c
extern char kernel_end;  // Address of kernel's end
void *end_addr = &kernel_end;
```

## Verification

After building, verify the layout:

```bash
readelf -l kernel.elf
```

You should see sections starting at `0x100000` with proper alignment.

---

**Next: [The CMake Build System →](build-system.md)**
