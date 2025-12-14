# Chapter 1: First Boot

> *"There are two ways of constructing a software design: One way is to make it so simple that there are obviously no deficiencies, and the other way is to make it so complicated that there are no obvious deficiencies."*  
> — C.A.R. Hoare

We have our tools installed. Time to write some code that talks directly to hardware. No operating system beneath us. No safety net. Just us, some assembly, and blind faith that GRUB will cooperate.

In this chapter, we'll:

- Set up the TinyOS project structure
- Write a Multiboot2-compliant bootloader header
- Create the boot entry assembly code
- Understand what remains to complete the boot process

By the end of this chapter, we'll have the foundation for a bootable kernel.

## Project Structure

Before we write a single line of assembly, let's talk organization. A well-structured OS project is the difference between "I know exactly where that code lives" and "I wrote this three weeks ago and now I can't find anything."

> **The Crux: Why Bother with Structure?**
>
> You might be tempted to throw everything in one directory and call it a day. Don't. In a few chapters, you'll have boot code, memory managers, device drivers, and interrupt handlers. Without clear boundaries, you'll end up with a 10,000-line `kernel.c` that nobody (including future you) can navigate.

TinyOS follows a modular, library-based design inspired by the Linux kernel and LLVM:

```
TinyOS/
├── boot/                   # Boot module
│   ├── multiboot.asm      # Multiboot2 header
│   ├── boot.asm           # Early boot assembly
│   └── CMakeLists.txt     # Boot build config
├── kernel/                 # Core kernel module
│   ├── main.c             # Kernel entry point (TODO)
│   ├── linker.ld          # Memory layout (TODO)
│   └── CMakeLists.txt     # Kernel build config (TODO)
├── arch/                   # Architecture abstraction
│   └── x86_64/            # x86-64 implementation
│       ├── include/
│       │   └── arch/      # Architecture headers
│       ├── io.c           # Port I/O (TODO)
│       └── serial.c       # Serial driver (TODO)
├── include/                # Standard library headers (freestanding)
│   ├── stdint.h           # Fixed-width types (uint32_t, etc.)
│   ├── stddef.h           # Basic definitions (size_t, NULL)
│   ├── stdbool.h          # Boolean type (bool, true, false)
│   └── tinyos/            # TinyOS-specific kernel headers
│       └── types.h        # Kernel semantic types
├── cmake/                  # Build system modules
│   ├── TinyOSPlatform.cmake
│   └── TinyOSHelpers.cmake
└── CMakeLists.txt          # Root build config
```

### The Tiny Standard Library

Since we're in freestanding mode (no host OS), we can't use the system's C standard library. We provide our own with familiar names:

**Standard headers** (`include/`):

- `stdint.h` — Fixed-width types (`uint32_t`, `int8_t`, etc.)
- `stddef.h` — Basic definitions (`size_t`, `NULL`, `offsetof`)
- `stdbool.h` — Boolean type (`bool`, `true`, `false`)

**Why standard names?** Familiarity. Every C programmer knows `uint32_t` and `size_t`. Using standard interfaces means:

- Less mental overhead
- Better IDE autocomplete  
- Easier to apply existing C knowledge

**TinyOS-specific types** (`tinyos/types.h`) add semantic meaning:

```c
#include <stdint.h>
#include <tinyos/types.h>

phys_addr_t addr = 0x100000;  // "This is a physical address"
uint64_t count = 256;         // "This is just a number"
```

The type system prevents mistakes. You can't accidentally pass a physical address where a virtual address is expected—the compiler catches the type mismatch.

### Module Patterns

Each module is self-contained with:

- Its own `CMakeLists.txt`
- An `include/` directory for public headers
- Implementation files

We won't explain CMake details—you should already know `add_library()`, `target_link_libraries()`, and directory structure conventions.

## The Multiboot2 Header

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

### The Multiboot2 Contract

Multiboot2 defines:

1. **Header format**: Magic number, architecture, checksum
2. **Boot state**: CPU mode, registers, memory state when we're loaded
3. **Information tags**: What data the bootloader provides (memory map, etc.)

Create `boot/multiboot.asm`:

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
    dd 8                    ; Request: framebuffer info
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

### Why This Matters

Without this header, GRUB won't recognize our kernel. The header is a contract: "I'm a valid kernel, here's what I need from you."

## Boot Entry Assembly

GRUB has loaded our kernel and jumped to our entry point. We're running! Sort of.

Here's our situation:

- `EAX` = Multiboot2 magic (`0x36d76289`) — proof GRUB loaded us
- `EBX` = Physical address of Multiboot info — memory map, bootloader name, etc.
- Interrupts disabled — no timer ticks interrupting us
- Paging disabled — physical addresses = virtual addresses (for now)
- **No stack configured** — uh oh

That last one is a problem. Without a stack, we can't:

- Call C functions (they need stack frames)
- Use local variables (stored on the stack)
- Handle interrupts (CPU pushes state to stack)
- Do basically anything useful

> **The Crux: Why Doesn't GRUB Set Up a Stack?**
>
> GRUB *could* set up a stack for us, but then we'd be tied to GRUB's choices. What if we want a bigger stack? Or a guard page to catch overflows? By forcing us to set it up ourselves, Multiboot2 gives us complete control.

Our first job: create a stack from scratch.

Create `boot/boot.asm`:

```nasm
global _start
extern kernel_main

section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

section .text
bits 32                     ; We're in 32-bit mode

_start:
    ; Ensure interrupts are disabled
    cli
    
    ; Set up stack pointer
    ; Stack grows downward, so ESP points to top
    mov esp, stack_top
    
    ; Reset EFLAGS to known state
    push 0
    popf
    
    ; Clear direction flag for string operations
    cld
    
    ; Save Multiboot info for kernel
    ; C calling convention: arguments pushed right-to-left
    push ebx                ; Multiboot info pointer (arg 2)
    push eax                ; Multiboot magic (arg 1)
    
    ; Call C kernel entry point
    ; void kernel_main(uint32_t magic, void *mbi)
    call kernel_main
    
    ; If kernel returns (shouldn't happen), halt
.hang:
    cli
    hlt
    jmp .hang
```

### Stack Setup Explained

Stacks are weird if you've never thought about them. They grow *downward* in memory. Why? Historical reasons involving PDP-11 computers from the 1970s. We're stuck with it.

The `.bss` section is zero-initialized memory ("Block Started by Symbol," if you're into archaic acronyms). We reserve 16 KiB there for our stack:

```
High Address  ┌──────────────┐ ← stack_top (ESP starts here)
              │              │
              │   16 KiB     │ Stack grows downward ↓
              │              │
Low Address   └──────────────┘ ← stack_bottom
```

When we `push`, ESP decrements. When we `pop`, ESP increments. The stack must be set up before calling any C function.

### Why 16 KiB?

Because it's a nice round number (in binary: `0x4000`) and it's enough space for our simple kernel initialization. Not so big we waste memory, not so small we overflow and crash in mysterious ways.

Later, when we implement proper process management, each process gets its own multi-page stack with guard pages to catch overflows. But for now? 16 KiB of blind optimism.

**Want to see how little stack space we actually need?** After we get the build system working, try reducing it to 8 KiB, then 4 KiB. Watch where it breaks. That's how you learn what actually consumes stack space in kernel initialization.

<!-- TODO: Add handdrawn illustration of stack frame layout showing function calls and local variables -->

## The Linker Script

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

### Memory Layout Strategy

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

The first 1MB is a minefield of BIOS tables, video memory, and historical baggage. Loading at 1MB gives us a clean slate.

### Section Organization

Our kernel has several sections, and their order matters:

1. **`.multiboot`** — GRUB scans the first 32KB for this. Must be first!
2. **`.text`** — Executable code (read-only, executable)
3. **`.rodata`** — Read-only data (string literals, const variables)
4. **`.data`** — Initialized global/static variables
5. **`.bss`** — Uninitialized data (zeroed at boot, saves space in binary)

Create `kernel/linker.ld`:

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

### Linker Script Explained

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

### What Happens During Linking

When you run the linker:

```bash
ld -T kernel/linker.ld -o kernel.elf boot.o main.o
```

The linker:

1. Reads all `.o` files (object files from assembler/compiler)
2. Follows the linker script to arrange sections
3. Resolves symbols (makes sure `call kernel_main` points to the right address)
4. Outputs a single `kernel.elf` binary

**Verify the layout:**

```bash
readelf -l kernel.elf
```

You should see sections starting at `0x100000` with proper alignment.

## What We've Built

Let's take stock. We now have:

- **Project structure** — directories organized, modules separated
- **Multiboot2 header** — the secret handshake GRUB needs
- **Boot assembly** — stack setup, register preservation, C function call
- **Linker script** — memory layout and section organization

Not bad! We can now assemble and link our boot code. Of course, we can't actually *boot* yet. For that, we still need:

- A build system (CMake + Ninja to orchestrate compilation)
- A kernel entry point (the C function `kernel_main()` we're calling)
- Serial output (so we can see that it works!)

**Current capabilities:**

- GRUB can recognize our kernel (if we could link it)
- We have a stack ready for C code
- We preserve Multiboot information for later use
- We know exactly where in memory everything lives
- We gracefully halt if the kernel returns

**Still missing:**

- Build automation
- Any visible output
- Memory management
- Interrupt handling

But we're standing on a solid foundation. Everything else builds from here.

## What's Next

To complete the boot process, we still need:

### CMake Build System (TODO)

Configure the build:

- Root `CMakeLists.txt` orchestrating modules
- `cmake/TinyOSPlatform.cmake` for compiler flags (`-ffreestanding`, `-mno-red-zone`, etc.)
- Module-specific `CMakeLists.txt` files

### Minimal Kernel Entry (TODO)

Implement `kernel/main.c`:

```c
void kernel_main(uint32_t magic, void *mbi) {
    // Verify Multiboot magic
    // Initialize basic services
    // Halt
}
```

### Serial I/O (TODO)

Add `arch/x86_64/serial.c` for COM1 output so we can debug.

### ISO Creation (TODO)

Script to create bootable ISO with GRUB.

### QEMU Testing (TODO)

Boot in QEMU and verify with LLDB debugging.

### VGA Text Mode (TODO)

Optional: Add screen output instead of just serial.

## Current State

You can assemble the boot files manually to see the raw output:

```bash
nasm -f elf64 boot/multiboot.asm -o multiboot.o
nasm -f elf64 boot/boot.asm -o boot.o
```

Want to see what we actually created? Use `objdump`:

```bash
objdump -D multiboot.o
```

You'll see the Multiboot header in raw hex—the `e8 50 52 d6` magic number (little-endian `0xe85250d6`) that GRUB searches for. This is what bootable looks like at the byte level.

Without a linker script and build system, we can't create a bootable kernel yet. That's next.

---

**Next: [Chapter 2: Physical Memory →](chapter-03-physical-memory.md)** *(TODO: Complete after boot process is working)*
