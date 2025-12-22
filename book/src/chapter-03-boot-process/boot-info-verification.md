# Boot info verification

Before we dive deeper into the creation of our kernel and start using the data GRUB handed us, there's something we should have done already but glossed over: **actually checking that Multiboot info is valid**.
This is needed to ensure that we can use the data in it later. This data can include:

* Memory map: Usable and reserved RAM regions (for memory management)
* Framebuffer info: For graphics output (if requested)
* Boot device: Which disk/partition was used to boot
* Command line: Kernel command-line arguments
* Modules: Loaded modules (e.g., initrd, drivers)
* ACPI tables: For power management and hardware enumeration
* Bootloader name/version
* ELF section headers: For kernel symbol table (useful for debugging)

[!side]
This is a pattern you'll see throughout kernel development: trust nothing, verify everything. Hardware lies. Bootloaders have bugs. Cosmic rays flip bits.
[/!side]

Right now, we're trusting that GRUB gave us good data in `magic` and `info`. That's... optimistic. What if GRUB didn't boot us? What if something went wrong? What if we're running on different hardware that uses a different bootloader?

> **The Crux: Defense Against the Dark Arts of Bootloaders**
>
> We need to verify:
>
> * **Magic number** - Is this actually Multiboot2 info?
> * **Total size** - Is the structure complete?
> * **Tag integrity** - Can we safely parse the tag list?
>
> Without these checks, we're one bad boot away from reading garbage memory and triple-faulting spectacularly.

Let's add proper validation before the kernels entry point starts using this data.

First we verify we were loaded by a Multiboot2-compliant bootloader. If not we will just halt the system for now since we can't print any error message at this point.

```c-diff
file: kernel/main.c
after: after function entry point
---
void kernel_main(uint32_t magic, void * info)
{  
+   if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
+       for (;;) {
+           __asm__ volatile("cli; hlt");
+       }
+   }
    for (;;) {
        __asm__ volatile("hlt");
    }
 }
```

Then we ensure that the multiboot info pointer is valid. If not we will simply halt the system as well.

```c-diff
file: kernel/main.c
after: after magic number verification
---
              __asm__ volatile("cli; hlt");
       }
   }
+    if (info == 0) {
+        // Invalid multiboot info, hang
+        for (;;) {
+            __asm__ volatile("cli; hlt");
+        }
+    }
    for (;;) {
        __asm__ volatile("hlt");
    }
```

## Verification

Now lets start qemu in debug mode and use lldb to have a look at the flow of the program.
First we need to recompile, then create a iso run qemu in debug mode and connect to it using lldb.

> TODO: Show program flow. Introduce frame variable.

## After bug discovery

## What Went Wrong?

GRUB by default starts in 32bit protected mode and we need to handle the transition to 64 bit.

## Lessons Learned

This is a valuable lesson in OS development:

* **Use a debugger!** What looks like "it works" might be "it's stuck in a halt loop."
* **Calling conventions matter.** You can't just call functions—you have to pass arguments correctly.
* **GRUB2 is helpful.** Modern GRUB with Multiboot2 handles mode transitions automatically.
* **Read the ABI spec.** System V AMD64 ABI defines how to call functions on x86-64.

> **Why Learn the Mode Transition if GRUB Does It?**
>
> You might wonder: "GRUB2 already transitions to 64-bit mode. Why learn how it works?"
>
> Because **understanding the transition teaches you fundamental OS concepts** you'll need later:
>
> * CPU operating modes and their constraints
> * Page table structure and virtual memory setup
> * Control registers (CR0, CR3, CR4) and MSRs
> * The relationship between paging and long mode
> * GDT structure and segment selectors
>
> These concepts are essential for memory management (Chapter 5), context switching, and system calls. GRUB's convenience hides them, but you need to understand the machinery to build a real kernel.
>
> Think of it like learning to drive a manual transmission—even if modern cars are automatic, understanding how the clutch and gears work makes you a better driver.

### Retesting

Open two terminals. In the first terminal, start QEMU with debugging enabled:

Rebuild the project then open the debugger, then instruct lldb to connect to QEMU's debbiging port to check the result again.

### Setting a Breakpoint

Tell LLDB to pause when we enter `kernel_main`:

```
(lldb) b kernel_main
Breakpoint 1: where = kernel.elf`kernel_main + 15 at main.c:42:15
```

[!side]
Breakpoints work by replacing the instruction at that address with a special INT3 instruction that traps to the debugger.
[/!side]

**What this means:** LLDB found the `kernel_main` function in our kernel at line 42 of main.c. When execution reaches that address, LLDB will pause the CPU.

### Running Until the Breakpoint

Now let the kernel boot:

```
(lldb) c
Process 1 resuming
Process 1 stopped
* thread #1, stop reason = breakpoint 1.1
    frame #0: 0x000000000010109f kernel.elf`kernel_main(magic=920085129, info=0x00000000001010e0) at main.c:42:15
   39   void kernel_main(uint32_t magic, struct multiboot_info * info)
   40   {
   41       // Verify we were loaded by a Multiboot2-compliant bootloader
-> 42       if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
   43           // Can't print error yet - just halt
```

### Inspecting the Multiboot Magic Number

Let's verify GRUB passed us the correct magic number. The `$rdi` register holds the first function argument (the `magic` parameter):

```
(lldb) p/x variable magic
(uint64_t) $0 = 0x0000000036d76289
```

**What this shows:** `p/x` means "print in hexadecimal." `variable magic` is used to specify the the variable. The value `0x36d76289` is the Multiboot2 magic number! GRUB loaded us correctly.

Let's check the multiboot info pointer in `info` (second argument):

```
(lldb) p/x variable info
(uint64_t) $1 = 0x00000000001010e0
```

That's a valid address pointing to the Multiboot information structure GRUB created for us.

### Stepping Through the Code

Let's watch the magic number check execute:

```
(lldb) n
Process 1 stopped
* thread #1, stop reason = step over
    frame #0: 0x00000000001010a9 kernel.elf`kernel_main(magic=920085129, info=0x00000000001010e0) at main.c:51:15
   48       }
   49   
   50       // Verify multiboot info pointer is valid
-> 51       if (info == NULL) {
```

**What happened?** `n` means "next" (step to the next line). The magic check passed (since the value was correct), so execution moved to line 51.

Step again to check the null pointer:

```
(lldb) n
Process 1 stopped
* thread #1, stop reason = step over
    frame #0: 0x00000000001010b9 kernel.elf`kernel_main(magic=920085129, info=0x00000000001010e0) at main.c:61:12
   58       
   59   
   60       // Kernel initialization complete - infinite loop for now
-> 61       for (;;) {
```

The null check passed too! Now we're at the infinite loop where the kernel halts.

### Viewing All Registers

Want to see the full CPU state?

```
(lldb) register read
General Purpose Registers:
       rax = 0x0000000036d76289
       rbx = 0x0000000000000000
       rcx = 0x0000000000000000
       rdx = 0x0000000000000535
       rdi = 0x0000000036d76289  ← magic number
       rsi = 0x00000000001010e0  ← multiboot info
       rbp = 0x0000000000000000
       rsp = 0x0000000000105000  ← stack pointer
       ...
```

You can see:

* `rsp` points to our stack (around 0x105000)
* `rdi` and `rsi` still hold the function arguments
* The CPU is in 64-bit mode (using 64-bit registers)

## What We Proved

Using LLDB, we verified:

1. **GRUB loaded our kernel** - The Multiboot2 magic number is correct
2. **Boot assembly executed** - Stack is set up, registers are correct
3. **64-bit transition worked** - CPU is in long mode with 64-bit registers
4. **C code is running** - We hit breakpoints and stepped through C functions
5. **Parameters are correct** - GRUB passed valid magic number and info pointer

**Your kernel is working.** The blank screen isn't a bug—it's exactly what we programmed it to do. We haven't written any video or serial output code yet, so there's nothing to display. But under the hood, the kernel booted successfully, verified the bootloader, and entered its main loop.

We need to add setup the stack to follow the calling convention of the System V AMD64 ABI .
Additionally it expects the first two arguments in RDI and RSI. Not EAX and EBX.
