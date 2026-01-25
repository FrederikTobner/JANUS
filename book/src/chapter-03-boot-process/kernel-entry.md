# The Kernel Entry Point

We have created the boot assembly that calls `kernel_main()` and the build system that links everything together. Time to write the actual C code that runs when the kernel starts.

This is it—the moment where all that assembly setup pays off and we finally get to write normal C code. Well, "normal" is relative. We're still in a freestanding environment with no standard library, but at least we're not juggling registers and segment selectors anymore.

When our boot assembly calls `kernel_main()`, here's what we're guaranteed:

```
    Assembly World      │  Transition  │     C World
      (boot.asm)        │              │  (kernel_main)
                        │              │
  32-bit registers      │  Preserved:  │   64-bit registers
  ┌─────────────┐       │              │     ┌────────────┐
  │ EAX = magic │───────┼──────────────┼───▶ │ RDI = magic│  first arg
  │ EBX = info  │───────┼──────────────┼───▶ │ RSI = info │  second arg
  └─────────────┘       │              │     └────────────┘
                        │              │
  Stack:                │              │  Stack:
  ESP ───▶ top          │              │  RSP ───▶ same location
                        │              │  (16 KiB available)
                        │              │
  CPU Mode:             │              │  CPU Mode:
  32-bit Protected      │  Switched!   │  64-bit Long Mode 
                        │              │
  Paging: DISABLED      │              │  Paging: ENABLED
                        │              │
  Interrupts: DISABLED  │              │  Interrupts: DISABLED
                        │              │  
```

> **System V AMD64 ABI Calling Convention**:
> First 6 integer arguments go in: RDI, RSI, RDX, RCX, R8, R9
> Our boot.asm preserves EDI and ESI through the mode switch,
> which become RDI and RSI in 64-bit mode. Perfect!

[!side]
Windows uses a different ABI. We follow System V because it's the Unix standard and therefore used by Linux, macOS, and BSD.
Under Windows only the first four arguments are passed using registers, everything else goes on the stack.
Under Unix systems the first six arguments go in registers, before the stack is utilized, which is more efficient.
[/!side]

We expect to have the Multiboot2 magic number (`0x36d76289`) in RDI and the physical address of the Multiboot information structure in RSI (was EBX). Additionally the stack should be set up with 16 KiB of space, and interrupts should be disabled.

> **Aside: Interrupts and the IF Flag**
>
> **Interrupts** are signals that pause normal execution to handle events (keyboard press, timer tick, hardware errors). The **IF (Interrupt Flag)** in the CPU's FLAGS register controls whether interrupts are enabled. When IF=0 (cleared), the CPU ignores maskable interrupts. GRUB disables interrupts before calling our kernel because we haven't set up interrupt handlers yet—if an interrupt fired, the CPU wouldn't know where to jump and would triple fault.

Also the CPU is in 64-bit long mode with paging enabled (identity-mapped first 2MB) and all segment registers cleared for a flat memory model.
But there are also many things that are currently not available to us, since we have only setup a minimal environment at this point in time, compared to what we are used to in normal C programming.
There is no heap setup yet, and we are freestanding, meaning no standard library is available.
Additionally we have no error handling beyond halting the system.

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

Let's create the main entry point of our kernel:

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

[!side]
The `volatile` keyword tells the compiler: "Don't optimize this away, even though it looks useless."
Without it, an optimizing compiler might think "this loop does nothing" and remove it. We *want* the infinite loop.
Compilers are too smart for their own good sometimes.
[/!side]

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

[!side]
Think of the `hlt` instruction as Gandalf standing in front of the CPU and screaming "You shall not pass!" until an interrupt comes along.
[/!side]

The `cli` instruction disables interrupts. This is redundant for now, but since we will enable interrupts later on, it's a good practice to ensure they are disabled before halting.
The `hlt` puts the CPU to sleep until the next interrupt. Since interrupts are disabled, this halts the CPU permanently.

Now let's create the `kernel/core/CMakeLists.txt` file in order to build the final executable.

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

This uses the `tinyos_create_kernel()` helper function (defined in `cmake/TinyOSHelpers.cmake` to:

- Link boot assembly objects with `main.c`
- Apply the custom linker script
- Produce the final `kernel.elf` executable

---

**Next: [Creating a Bootable ISO →](creating-bootable-iso.md)**
