# The Kernel Entry Point

We have boot assembly that calls `kernel_main()`. We have a build system that links everything together. Time to write the actual C code that runs when the kernel starts.

> **The Crux: Why Verify the Magic Number?**
>
> GRUB passes a magic number (`0x36d76289`) in the `EAX` register to prove it loaded us correctly. If this number is wrong, something went catastrophically wrong—maybe we weren't loaded by a Multiboot2 bootloader at all. Better to catch this early and halt than to proceed with invalid state.

## The Handoff from Assembly to C

When our boot assembly calls `kernel_main()`, here's what we're guaranteed:

**Register state:**

- `EAX` = Multiboot2 magic number (`0x36d76289`)
- `EBX` = Physical address of Multiboot information structure
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

Create `kernel/main.c`:

```c
// kernel/main.c
#include <stdint.h>
#include <stddef.h>
#include <boot/multiboot.h>

void kernel_main(uint32_t magic, void *info)
{
    // Verify we were loaded by a Multiboot2-compliant bootloader
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        // Magic number mismatch - halt
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    // Verify multiboot info pointer is valid
    if (info == NULL) {
        // Null pointer - halt
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }
    
    // At this point:
    // - Bootloader magic verified
    // - Multiboot info pointer is valid
    // - We're ready to start kernel initialization

    // TODO: Initialize serial output
    // TODO: Parse multiboot information
    // TODO: Set up memory management
    // TODO: Enable interrupts

    // Kernel initialization complete - infinite loop for now
    for (;;) {
        __asm__ volatile("hlt");
    }
}
```

## What This Does

**Magic number check:** If GRUB didn't load us, `magic` will be garbage. We halt immediately rather than continuing with corrupted state.

**Null pointer check:** Defense against bootloader bugs. The Multiboot spec says `EBX` contains a valid pointer, but we verify anyway.

**The halt loops:** `cli` disables interrupts (redundant, but explicit). `hlt` puts the CPU to sleep until the next interrupt. Since interrupts are disabled, this halts the CPU permanently.

## Why `volatile`?

```c
__asm__ volatile("hlt");
```

The `volatile` keyword tells the compiler: "Don't optimize this away, even though it looks useless." Without it, an optimizing compiler might think "this loop does nothing" and remove it. We *want* the infinite loop.

## Testing the Kernel

Let's build and verify:

```bash
ninja -C build
```

Check that `kernel_main` symbol exists:

```bash
nm build/kernel.elf | grep kernel_main
```

You should see:

```
0000000000101090 T kernel_main
```

The `T` means it's in the text (code) section, and the address shows where it lives in memory.

At this point, the kernel boots, verifies it was loaded correctly, and halts. Not exciting, but it's correct! We've established the foundation for everything else.

**What happens if you boot this kernel?**

- GRUB loads it and jumps to `_start`
- Boot assembly sets up stack and calls `kernel_main`
- `kernel_main` verifies magic number
- Kernel halts (CPU sleeps forever)

**What you'll see:** Nothing. The screen stays blank because we haven't initialized any output yet. But QEMU won't crash, and a debugger would show we're sitting in that `hlt` instruction. Success!

## Creating the Kernel Build File

Create `kernel/CMakeLists.txt` to build the final executable:

```cmake
# kernel/CMakeLists.txt

# Create kernel executable with linker script
tinyos_create_kernel(
    SOURCES main.c
    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
    LIBRARIES ${BOOT_OBJECTS}
)
```

This uses the `tinyos_create_kernel()` helper function (defined in `cmake/TinyOSHelpers.cmake`) to:

- Link boot assembly objects with `main.c`
- Apply the custom linker script
- Produce the final `kernel.elf` executable

---

**Next: [Creating a Bootable ISO →](creating-bootable-iso.md)**
