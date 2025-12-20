# Boot Entry Assembly

GRUB finds our Multiboot header, validates it, and jumps to our entry point. But what state does GRUB leave the CPU in? The answer might surprise you.

## What Does GRUB Give Us?

With Multiboot2 and a 64-bit ELF kernel, GRUB2 is surprisingly helpful. When it detects we're loading a 64-bit kernel, it:

- **Already transitions to 64-bit long mode** for us
- Sets up basic identity-mapped page tables
- Loads a 64-bit GDT
- Puts the CPU in 64-bit long mode

When control reaches our entry point:

- **EAX** = Multiboot2 magic number (0x36d76289)
- **EBX** = Physical address of Multiboot information structure
- **EIP** = Our entry point (but we're in 64-bit mode, so it's really RIP!)
- CPU is **already in 64-bit long mode**
- Everything else is undefined (stack, other registers, interrupts)

[!side]
**Wait, GRUB does all that for us?**

Yes! Modern GRUB2 with Multiboot2 detects 64-bit ELF kernels and handles the mode transition automatically. Older GRUB (pre-2.0) or Multiboot1 started in 32-bit mode and required manual transition. We'll learn the manual transition in the next chapter because understanding it teaches you how the CPU actually works—but for now, GRUB's doing the heavy lifting.
[!side]

## The Naive Approach

Let's try the simplest possible boot code:

Our plan:

1. Set up a stack
2. Preserve the Multiboot parameters
3. Call `kernel_main`

Since GRUB already put us in 64-bit mode, let's write simple 64-bit boot code:

1. Set up a stack
2. Call `kernel_main`

Create `boot/boot.asm`:

```x86asm-diff
file: boot/boot.asm
---
+; Boot entry point - GRUB puts us in 64-bit long mode
+global _start
+extern kernel_main
+
+section .bss
+align 16
+stack_bottom:
+    resb 16384              ; 16 KiB stack
+stack_top:
+
+section .text
+bits 64                     ; We're already in 64-bit mode!
+
+_start:
+    call kernel_main
+    
+.hang:
+    cli
+    hlt
+    jmp .hang
```

**What this code does:**

1. **Stack setup**: Point `RSP` to our 16 KiB stack (\\(16384 = 2^{14}\\) bytes)
2. **Call kernel**: Jump to our C code (but with wrong calling convention!)
3. **Halt loop**: If `kernel_main` returns, halt the CPU

Looks reasonable. Let's add it to our CMake build and try it.

## Building the Kernel

Update `boot/CMakeLists.txt`:

```cmake-diff
file: boot/CMakeLists.txt
replace: entire file
---
+# Boot assembly - Entry point from GRUB
+enable_language(ASM_NASM)
+set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
+
+set(BOOT_ASM_SOURCES
+    multiboot.asm
+    boot.asm
+)
+
+foreach(ASM_FILE ${BOOT_ASM_SOURCES})
+    get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
+    set(ASM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${ASM_NAME}.o)
+    
+    add_custom_command(
+        OUTPUT ${ASM_OBJ}
+        COMMAND nasm -f elf64 -o ${ASM_OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_FILE}
+        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ASM_FILE}
+        COMMENT "Assembling ${ASM_FILE}"
+    )
+    
+    list(APPEND BOOT_OBJECTS ${ASM_OBJ})
+endforeach()
+
+set(BOOT_OBJECTS ${BOOT_OBJECTS} PARENT_SCOPE)
```

Build and create the ISO:

```bash
cmake -B build -G Ninja
ninja -C build iso
```

---

**Next: [Understanding the 64-bit Transition](boot-assembly-64bit.md)**
