# Boot Entry Assembly

GRUB finds our Multiboot header, validates it, and jumps to our entry point. 
To transfer control from GRUB to our C code we need a small assembly stub that sets up the stack and calls our C entry point.


Create `kernel/boot/boot.asm`:

```x86asm-diff
file: kernel/boot/boot.asm
---
+global _start
+extern kernel_main
+
+section .bss
+align 16
+stack_bottom:
+    resb 16384              
+stack_top:
+
+section .text
+bits 64                     
+
+_start:
+    call kernel_main
+    
+.hang:
+    cli
+    hlt
+    jmp .hang
```

Fist we declare the global `_start` label, which is where GRUB will jump to after loading our kernel. We also declare an external symbol `kernel_main`, which will be the main entry point of our kernel code hat has been written in C. Later we setup the stack in the `.bss` section, reserving 16 KiB for it. In the `.text` section, we define the `_start` label, which simply calls `kernel_main`. After `kernel_main` returns (which it shouldn't), we enter an infinite loop that halts the CPU. This is only a defensive measure to ensure the CPU doesn't execute random instructions if `kernel_main` were to return.

[!side]
When working on a kernel, it is crucial to be very defensive, you could even say almost paranoid. 
Like what happens if my kernel, has a broken BIOS, or a cosmic ray flips a bit in your boot info? 
Always expect the unexpected!
[/side]

## Building the Kernelqemu x86 virtualization

Update `kernel/boot/CMakeLists.txt`:

```cmake-diff
file: kernel/boot/CMakeLists.txt
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

**Next: [The Linker Script](linker-script.md)** 
