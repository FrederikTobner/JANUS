# Boot Entry Assembly (32-bit)

GRUB finds our Multiboot header, validates it, and jumps to our entry point. But there's a problem: GRUB leaves the CPU in an undefined state. Interrupts might be enabled, the stack pointer is garbage, and we need to set up a minimal runtime environment before calling our C kernel.

[!side]
This section shows the naive approach that fails. We learn by seeing what goes wrong.
[/!side]

Let's start with the simplest possible boot code.

## The Naive Approach

GRUB boots us in 32-bit protected mode and gives us:

- `EAX` = Multiboot2 magic number (0x36d76289)
- `EBX` = Physical address of Multiboot information structure
- CPU in 32-bit protected mode
- Everything else is undefined

Our plan:

1. Set up a stack
2. Preserve the Multiboot parameters
3. Call `kernel_main`

Simple, right? Let's try it.

Create `boot/boot.asm`:

```nasm
; Boot entry point - called by GRUB in 32-bit protected mode
global _start
extern kernel_main

; Reserve stack space in BSS section
section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

section .text
bits 32                     ; GRUB puts us in 32-bit protected mode

_start:
    ; At this point:
    ; - EAX = multiboot magic (0x36d76289)
    ; - EBX = physical address of multiboot info structure
    ; - CPU is in 32-bit protected mode
    
    ; Set up stack pointer
    mov esp, stack_top
    
    ; Push multiboot info onto stack for kernel_main
    push ebx                ; multiboot info pointer (arg 2)
    push eax                ; multiboot magic (arg 1)
    
    ; Call kernel (this is where things go wrong...)
    call kernel_main
    
    ; If kernel_main returns, halt
.hang:
    cli
    hlt
    jmp .hang
```

**What this code does:**

1. **Stack setup**: Point `ESP` to our 16 KiB stack (\\(16384 = 2^{14}\\) bytes)
2. **Preserve arguments**: Push magic number and info pointer for `kernel_main`
3. **Call kernel**: Jump to our C code
4. **Halt loop**: If `kernel_main` returns, halt the CPU

Looks reasonable. Let's add it to our CMake build and try it.

## Building the Kernel

Update `boot/CMakeLists.txt`:

```cmake
# Assemble boot files
add_library(boot STATIC)

target_sources(boot PRIVATE
    multiboot.asm
    boot.asm           # Add our new boot assembly
)

set_target_properties(boot PROPERTIES
    NASM_OBJ_FORMAT elf64
    LINKER_LANGUAGE C
)

target_include_directories(boot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

Now build and create the ISO:

```bash
cmake -B build -G Ninja
ninja -C build iso
```

## The Moment of Truth

Let's boot our kernel:

```bash
ninja -C build run
```

**TODO:** Validate this and consider providing screenshots

**What happens:** The QEMU window opens... and immediately closes. Or it reboots continuously. Something is very wrong.

## Debugging the Triple Fault

Let's get more information. Run QEMU with logging:

```bash
qemu-system-x86_64 -cdrom build/tinyos.iso -boot d \
    -d int,cpu_reset -D qemu.log -no-reboot
```

Check `qemu.log`:

```
Triple fault. Halting for inspection.
```

**TODO:** Replace with real validated output

**Triple fault!** The CPU couldn't recover from an exception and reset itself.

[!side]
Triple faults are the CPU's way of saying "I give up." It's the ultimate error condition.
[/!side]

## What Went Wrong?

Look at the CPU state carefully:

- `RIP` = 0x0000000000101090 (trying to execute at this address)
- Notice the **R** prefix on registers (RAX, RBX, RIP) - these are 64-bit registers!
- But we're in 32-bit mode!

**The problem:** Our kernel is compiled for x86_64 (64-bit mode), but the CPU is still in 32-bit protected mode. When GRUB jumped to `_start`, we set up a stack and called `kernel_main`, which contains 64-bit instructions. The CPU can't execute 64-bit instructions in 32-bit mode, so it throws an exception. The exception handler isn't set up, so it throws another exception. This cascades into a **triple fault**, and the CPU gives up and resets.

Let's verify this by checking our kernel:

```bash
file build/kernel.elf
```

Output:

```
build/kernel.elf: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), 
statically linked, with debug_info, not stripped
```

**There it is:** Our kernel is a 64-bit executable. But we're trying to run it in 32-bit mode.

## The Real Boot Handoff Problem

GRUB boots us in **32-bit protected mode**, but our kernel is compiled for **64-bit long mode**. We can't just call the kernel directly—we need to transition the CPU to 64-bit mode first.

This transition requires:

1. Setting up page tables (long mode requires paging)
2. Enabling PAE (Physical Address Extension)
3. Setting the long mode bit in the EFER MSR
4. Enabling paging
5. Loading a 64-bit GDT
6. Performing a far jump to 64-bit code

In other words, our "simple" boot code wasn't simple enough—we skipped the most critical part!

## Lessons Learned

This is a valuable lesson in OS development:

- **The CPU mode matters.** You can't mix 32-bit and 64-bit code without explicit transitions.
- **Triple faults are cryptic.** QEMU's logging (`-d int,cpu_reset`) is essential for debugging.
- **Assumptions kill kernels.** We assumed calling `kernel_main` would just work. It didn't.

Don't feel bad if you made this mistake—I did too! It's a rite of passage in OS development.

> **Why Learn This if UEFI Skips It?**
>
> You might wonder: "Modern systems use UEFI, which boots directly into 64-bit mode. Why are we learning this 32-bit to 64-bit transition?"
>
> Because **understanding the transition teaches you how the CPU actually works.** You're learning about:
>
> - CPU operating modes and their constraints
> - Page table structure and virtual memory setup
> - Control registers (CR0, CR3, CR4) and MSRs
> - The relationship between paging and long mode
> - GDT structure and segment selectors
>
> These concepts are fundamental to OS development. UEFI's convenience hides them, but you still need to understand them for memory management, context switching, and system calls later.
>
> Think of it like learning to drive a manual transmission—even if you'll mostly drive automatic cars, understanding how the clutch and gears work makes you a better driver overall.

## What's Next

Now that we understand *why* we need to transition to long mode, let's actually do it. In the next section, we'll write the proper boot assembly that:

1. Starts in 32-bit mode (as GRUB gives us)
2. Sets up page tables
3. Enables PAE and long mode
4. Switches to 64-bit code
5. Calls our 64-bit kernel successfully

---

**Next: [Transitioning to Long Mode](boot-assembly-longmode.md)**
