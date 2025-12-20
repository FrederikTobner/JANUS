# Creating a Bootable ISO

We have a complete kernel: Multiboot2 header, boot assembly, linker script, C entry point, and CMake build system. Everything compiles and links successfully producing a `kernel.elf` file. But how do we actually *boot* it?

Here's the problem: QEMU can't just boot our ELF file. It expects either a Linux-style bzImage or a special PVH boot protocol. Our kernel speaks Multiboot2, which requires a bootloader. Enter GRUB.

> **The Crux: Why Can't We Just Load the Kernel Directly?**
>
> You might wonder why we need this extra step. The kernel is right there! But real hardware doesn't know how to parse ELF files. GRUB acts as the middleman: it understands ELF, sets up the environment our kernel expects (64-bit mode, Multiboot info structure, etc.), and hands control over. Think of it as a helpful librarian who finds your book and opens it to the right page.

## Why an ISO?

An ISO image is a bootable CD/DVD image that contains:

- GRUB bootloader (the first code that runs)
- Our kernel executable
- GRUB configuration (the menu you see when booting)

[!side]
ISO 9660 is the CD-ROM filesystem standard from 1988. Still widely used because it works everywhere.
[/!side]

This is the standard way to distribute and test operating systems. Burn it to a USB drive and it'll boot on real hardware. Or just test it in QEMU—same image works everywhere.

## CMake Targets for ISO Creation

To automate ISO creation, we need to add custom CMake targets to the root `CMakeLists.txt`.

### Prerequisites

First, install the required GRUB tools:

```bash
# Ubuntu/Debian
sudo apt install grub-pc-bin xorriso mtools

# Arch Linux
sudo pacman -S grub xorriso mtools

# macOS
brew install grub xorriso mtools
```

### Adding the Targets

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

Then we setup a custom ninja command to create the iso.

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
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso -boot d -serial stdio
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
+    COMMAND qemu-system-x86_64 -cdrom ${CMAKE_BINARY_DIR}/tinyos.iso -boot d -serial stdio -s -S
+    DEPENDS iso
+    COMMENT "Running TinyOS in QEMU with GDB stub (waiting for debugger on :1234)"
+)
 
 message(STATUS "========================================")
```

**Overview:**

- **`iso`** - Creates the ISO directory structure, copies kernel, generates grub.cfg, and creates tinyos.iso
- **`run`** - Depends on `iso`, then boots the ISO in QEMU
- **`debug`** - Same as `run` but with GDB stub enabled on port 1234

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

**What changed?**

1. **GRUB config template** - `cmake/grub.cfg.in` is now version-controlled
2. **CMake generates grub.cfg** - Uses `configure_file()` at configure time
3. **ISO target simplified** - Just copies the generated file instead of echo commands
4. **Dependency tracking** - ISO rebuilds if grub.cfg changes

### Using the Targets

```bash
# Build kernel and create bootable ISO
ninja -C build iso
```

You'll see GRUB and xorriso working to create the bootable image:

```
ninja: Entering directory `build'
[1/1] Creating bootable ISO image
xorriso 1.5.4 : RockRidge filesystem manipulator, libburnia project.

Drive current: -outdev 'stdio:/home/user/TinyOS/build/tinyos.iso'
Media current: stdio file, overwriteable
Media status : is blank
Media summary: 0 sessions, 0 data blocks, 0 data, 80.0g free
Added to ISO image: directory '/'='/tmp/grub.xyz123'
xorriso : UPDATE :     587 files added in 1 seconds
Added to ISO image: directory '/'='/home/user/TinyOS/build/iso'
xorriso : UPDATE :     591 files added in 1 seconds
xorriso : NOTE : Copying to System Area: 512 bytes from file '/usr/lib/grub/i386-pc/boot_hybrid.img'
ISO image produced: 5747 sectors
Written to medium : 5747 sectors at LBA 0
Writing to 'stdio: TinyOS/build/tinyos.iso' completed successfully.
```

**What just happened?**

1. **Created directory structure** - `iso/boot/grub/`
2. **Copied kernel** - `kernel.elf` → `iso/boot/kernel.elf`
3. **Generated GRUB config** - `grub.cfg` with our boot menu
4. **Added GRUB modules** - 587 files (GRUB bootloader, modules, fonts)
5. **Added our kernel** - 4 more files (kernel + directory entries)

[!side]
GRUB is modular. Those 587 files include filesystem drivers, video modes, compression support, etc.
[/!side]
6. **Created hybrid boot** - Works with both BIOS (i386-pc) and UEFI (x86_64-efi)
7. **Wrote ISO** - Final image is ~11MB (5747 sectors × 2KB)

The result is `build/tinyos.iso` - a bootable ISO image that works on both BIOS and UEFI systems.

### What's in the ISO?

```
build/iso/
├── boot/
│   ├── kernel.elf          ← Our kernel
│   └── grub/
│       └── grub.cfg        ← GRUB configuration
```

The GRUB configuration (`grub.cfg`) is generated from `cmake/grub.cfg.in`:

```
set timeout=0
set default=0

menuentry "TinyOS" {
    multiboot2 /boot/kernel.elf
    boot
}
```

- `multiboot2 /boot/kernel.elf` - Load using Multiboot2 protocol
- `boot` - Start execution

## What's Next

We now have a bootable ISO image with our kernel. The build system automates everything:

- `ninja -C build iso` creates the ISO
- `ninja -C build run` boots it in QEMU
- `ninja -C build debug` enables debugging

But does it actually work? Let's find out.

---

**Next: [Booting Up](booting-up.md)**
