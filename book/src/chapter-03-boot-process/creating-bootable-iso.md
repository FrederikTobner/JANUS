# Creating a Bootable ISO

We have a complete kernel: Multiboot2 header, boot assembly, linker script, C entry point, and CMake build system. 
Everything compiles and links successfully producing a `kernel.elf` file. 
But how do we actually *boot* it?

Here's the problem: QEMU can't just boot our ELF file. It expects either a Linux-style bzImage or a special PVH boot protocol. Our kernel speaks Multiboot2, which requires a bootloader. This is where GRUB comes in - the GRand Unified Bootloader.

An ISO image is a bootable CD/DVD image that contains, the GRUB bootloader, our kernel, and GRUB configuration.

[!side]
ISO 9660 is the CD-ROM filesystem standard from 1988. Still widely used because it works everywhere.
[/!side]

This is the standard way to distribute and test operating systems.
Burn it to a USB drive and it'll boot on real hardware. Or just test it in QEMU—same image works everywhere.

To automate ISO creation, we need to add custom CMake targets to the root `CMakeLists.txt`.

First, install the required GRUB tools:

```bash
# Debian-based systems 
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools
```

The ISO creation uses a GRUB configuration template. First, create `cmake/grub.cfg.in`:

```cfg-diff
file: cmake/grub.cfg.in
replace: entire file
---
+set timeout=0
+set default=0
+
+menuentry "TinyOS" {
+    multiboot2 /boot/kernel.elf
+    boot
+}
```

Now open the root `CMakeLists.txt` and add this command to add the grub.cfg.in to the build directory after being processed by Cmake.

```cmake-diff
file: CMakeLists.txt
after: add_subdirectory(kernel)
---
 add_subdirectory(kernel)
+
+# Generate grub.cfg from template
+configure_file(
+    ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in
+    ${CMAKE_BINARY_DIR}/grub.cfg
+    @ONLY
+)
+
 message(STATUS "========================================")
```

Next we will setup a custom command to create the ISO image using `grub-mkrescue`.

```cmake-diff
file: CMakeLists.txt
after: configure_file using grub.cfg.in
---
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/grub.cfg.in
    ${CMAKE_BINARY_DIR}/grub.cfg
    @ONLY
)

# ISO creation target
+add_custom_target(iso
+    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
+    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf> ${CMAKE_BINARY_DIR}/iso/boot/kernel.elf
+    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
+    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/tinyos.iso ${CMAKE_BINARY_DIR}/iso/
+    DEPENDS kernel.elf ${CMAKE_BINARY_DIR}/grub.cfg
+    COMMENT "Creating bootable ISO image"
+)
+
 
 # Print build configuration
 message(STATUS "========================================")
```

This command does a couple of things. It creates the ISO directory structure, copies kernel, generates grub.cfg, and finally creates the tinyos.iso image.

Now lets add another custom command to boot the iso in QEMU. This command depends in iso.

```cmake-diff
file: CMakeLists.txt
after: add_custom_target(iso)
---
    COMMENT "Creating bootable ISO image"
)
+
+add_custom_target(run
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso 
+     -boot d 
+    DEPENDS iso
+    COMMENT "Running TinyOS in QEMU"
+)
+
 
 message(STATUS "========================================")
```

And another command to debug our application using a GDB stub enabled on port 1234. This command depends on iso as well.

```cmake-diff
file: CMakeLists.txt
after: add_custom_target(run)
---
    COMMENT "Running TinyOS in QEMU"
)
+
+add_custom_target(debug
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso 
+      -boot d 
+    DEPENDS iso
+    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
+)
 
 message(STATUS "========================================")
```

> **Overview:**
> 
> - `iso` - Creates the ISO directory structure, copies kernel, generates grub.cfg, and creates tinyos.iso
> - `run` - Depends on `iso`, then boots the ISO in QEMU
> - `debug` - Same as `run` but with GDB stub enabled on port 1234, allowing us to connect a debugger

Lets update the printed build configuration to show this:


```cmake-diff
file: CMakeLists.txt
after: printing build info
---
message(STATUS "========================================")
+message(STATUS "Available targets:")
+message(STATUS "  ninja           - Build kernel.elf")
+message(STATUS "  ninja iso       - Create bootable ISO")
+message(STATUS "  ninja run       - Build and run in QEMU")
+message(STATUS "  ninja debug     - Build and run with debugger")
+message(STATUS "========================================")
```

Reconfigure CMake to pick up the new targets:

```bash
cmake -B build -G Ninja
```

You'll see output showing the new targets:

```
-- ========================================
-- TinyOS Build Configuration
-- ========================================
-- C Standard: C17
-- Build Type: Debug
-- Compiler: /usr/local/bin/clang
-- Assembler: /usr/bin/nasm
-- ========================================
-- Available targets:
--   ninja           - Build kernel.elf
--   ninja iso       - Create bootable ISO
--   ninja run       - Build and run in QEMU
--   ninja debug     - Build and run with debugger
-- ========================================
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/user/TinyOS/build
```

Now lets start using the targets we have created.

[!side]
You can use the help command `ninja -C build help` to see all available targets.
Your new targets `iso`, `run`, and `debug` will be listed there as well as phony targets.
This does not mean that these targets are fake, it just means they don't prooduce an artifact with the same name.
Unlike phony politicians, phony targets in CMake are actually useful.
[/!side]

```bash
# Build kernel and create bootable ISO
ninja -C build iso
```

You'll see they creation of the ISO image in the output: 

```
ninja: Entering directory `build'
[1/1] Creating bootable ISO image
xorriso 1.5.6 : RockRidge filesystem manipulator, libburnia project.

Drive current: -outdev 'stdio:/home/user/TinyOS/build/tinyos_x86_64.iso'
Media current: stdio file, overwriteable
Media status : is blank
Media summary: 0 sessions, 0 data blocks, 0 data, 33.5g free
Added to ISO image: directory '/'='/tmp/grub.A9hghz'
xorriso : UPDATE :    1058 files added in 1 seconds
Added to ISO image: directory '/'='/home/user/TinyOS/build/iso'
xorriso : UPDATE :    1062 files added in 1 seconds
xorriso : NOTE : Copying to System Area: 512 bytes from file '/usr/lib/grub/i386-pc/boot_hybrid.img'
ISO image produced: 15591 sectors
Written to medium : 15591 sectors at LBA 0
Writing to 'stdio:/home/user/TinyOS/build/tinyos_x86_64.iso' completed successfully.
```

Lets reflect on what we have accomplished now, by running this command. 
First we create the directory structure for the ISO, then we copy our kernel and GRUB configuration into place, and finally we use `grub-mkrescue` to create the bootable ISO image.

The result is `build/tinyos_x86_64.iso` - a bootable ISO image that works on both BIOS and UEFI systems.

Next we will boot it up in QEMU to verify everything works as expected.

---

**Next: [Booting Up](booting-up.md)**
