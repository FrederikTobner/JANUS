# The Kernel Entry Point

We have created the boot assembly that calls `kernel_main()`and the build system that links everything together. Time to write the actual C code that runs when the kernel starts.

This is it—the moment where all that assembly setup pays off and we finally get to write normal C code. Well, "normal" is relative. We're still in a freestanding environment with no standard library, but at least we're not juggling registers and segment selectors anymore.

> **The Crux: Why Verify the Magic Number?**
>
> GRUB passes a magic number (`0x36d76289`) in the `EAX` register to prove it loaded us correctly. If this number is wrong, something went catastrophically wrong—maybe we weren't loaded by a Multiboot2 bootloader at all. Better to catch this early and halt than to proceed with invalid state.

## The Handoff from Assembly to C

When our boot assembly calls `kernel_main()`, here's what we're guaranteed:

### CPU State on Entry to kernel_main

```
    Assembly World  │  Transition  │     C World
   (boot.asm)       │              │  (kernel_main)
                    │              │
  32-bit regs:      │  Preserved:  │  64-bit regs:
  ┌──────────┐     │              │  ┌──────────┐
  │ EAX = magic│────┼─────────────▶│  │ RDI = magic│  (arg 1)
  │ EBX = info │────┼─────────────▶│  │ RSI = info │  (arg 2)
  └──────────┘     │              │  └──────────┘
                    │              │
  Stack:            │              │  Stack:
  ESP ───▶ top      │              │  RSP ───▶ same location
                    │              │  (16 KiB available)
                    │              │
  CPU Mode:         │              │  CPU Mode:
  Protected (32-bit)│  Switched!   │  Long (64-bit)
  Paging: OFF       │              │  Paging: ON
  Interrupts: OFF   │              │  Interrupts: OFF
                    │              │  (still disabled!)
```

> **System V AMD64 ABI Calling Convention**:
> First 6 integer arguments go in: RDI, RSI, RDX, RCX, R8, R9
> Our boot.asm preserves EDI and ESI through the mode switch,
> which become RDI and RSI in 64-bit mode. Perfect!

[!side]
Windows uses a different ABI (RCX, RDX, R8, R9). We follow System V because it's the Unix/Linux standard.
[/!side]

**Register state:**

- `RDI` (was `EAX`) = Multiboot2 magic number (`0x36d76289`)
- `RSI` (was `EBX`) = Physical address of Multiboot information structure
- Stack configured (16 KiB)
- Interrupts disabled (`IF` flag clear)

> **Aside: Interrupts and the IF Flag**
>
> **Interrupts** are signals that pause normal execution to handle events (keyboard press, timer tick, hardware errors). The **IF (Interrupt Flag)** in the CPU's FLAGS register controls whether interrupts are enabled. When IF=0 (cleared), the CPU ignores maskable interrupts. GRUB disables interrupts before calling our kernel because we haven't set up interrupt handlers yet—if an interrupt fired, the CPU wouldn't know where to jump and would triple fault.

**CPU mode:**

- 64-bit long mode (boot assembly transitions from 32-bit to 64-bit before calling us)
- Paging enabled (identity-mapped first 2MB)
- All segment registers cleared (flat memory model)

**What we're NOT guaranteed:**

- No heap (we haven't set one up yet)
- No standard library (we're freestanding)
- No error handling beyond halt

## Implementing kernel_main()

Let's build the kernel entry point step by step. Create an empty file:

```bash
# From project root
touch kernel/core/main.c
```

First we need to include some of the headers we have created in the previous chapters:

```c-diff
file: kernel/core/main.c
replace: entire file
---
+#include <stdint.h>
+#include <stddef.h>
+#include <boot/multiboot.h>
```

Lets create the main entry point of our kernel:

```c-diff
file: kernel/core/main.c
after: #include <boot/multiboot.h>
---
 #include <boot/multiboot.h>
+
+void kernel_main(u32 magic, void * info)
+{
+}
```

Our function takes:

- `magic` - The Multiboot2 magic number from RDI (was EAX in 32-bit mode)
- `info` - Pointer to Multiboot information from RSI (was EBX in 32-bit mode)

For now we will just do an infinite loop.

```c-diff
file: kernel/core/main.c
after: after function entry point
---
void kernel_main(uint32_t magic, void * info)
{  
+    for (;;) {
+        __asm__ volatile("hlt");
+    }
 }
```

## What This Does

**The halt loops:** `cli` disables interrupts (redundant, but explicit). `hlt` puts the CPU to sleep until the next interrupt. Since interrupts are disabled, this halts the CPU permanently.

[!side]
Compilers are too smart for their own good sometimes. `volatile` is our way  of saying "trust me on this one."
[/!side]

> **New to `volatile`?**
>
> ```c
> __asm__ volatile("hlt");
> ```
>
> The `volatile` keyword tells the compiler: "Don't optimize this away, even though it looks useless."
> Without it, an optimizing compiler might think "this loop does nothing" and remove it. We *want* the infinite loop.
>
>
>

## Creating the Kernel Build File

Create `kernel/core/CMakeLists.txt` to build the final executable:

```cmake-diff
file: kernel/CMakeLists.txt
after: entire file
---
+# Create kernel executable with linker script
+tinyos_create_kernel(
+    SOURCES main.c
+    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
+    LIBRARIES ${BOOT_OBJECTS}
+)
```

This uses the `tinyos_create_kernel()` helper function (defined in `cmake/TinyOSHelpers.cmake`) to:

- Link boot assembly objects with `main.c`
- Apply the custom linker script
- Produce the final `kernel.elf` executable

---

**Next: [Creating a Bootable ISO →](creating-bootable-iso.md)**
